open PesyUtils;
open Utils;
open NoLwt;
open Printf;
module Mode = Mode;
module PesyConf = PesyConf;

let bootstrapIfNecessary = projectPath =>
  if (!isEsyInstalled()) {
    fprintf(
      stderr,
      "ERROR: You haven't installed esy globally. First install esy then try again",
    );
  } else {
    let packageNameKebab = kebab(Filename.basename(projectPath));
    let packageNameKebabSansScope = removeScope(packageNameKebab);
    let packageNameUpperCamelCase =
      upperCamelCasify(packageNameKebabSansScope);
    let version = "0.0.0";
    let packageJSONTemplate = loadTemplate("pesy-package.template.json");
    let appReTemplate = loadTemplate("pesy-App.template.re");
    let testReTemplate = loadTemplate("pesy-Test.template.re");
    let utilRe = loadTemplate("pesy-Util.template.re");
    let readMeTemplate = loadTemplate("pesy-README.template.md");
    let gitignoreTemplate = loadTemplate("pesy-gitignore.template");
    let packageLibName = packageNameKebabSansScope ++ ".lib";

    if (!exists("package.json")) {
      let packageJSON =
        packageJSONTemplate
        |> Str.global_replace(r("<PACKAGE_NAME_FULL>"), packageNameKebab)
        |> Str.global_replace(r("<VERSION>"), version)
        |> Str.global_replace(r("<PUBLIC_LIB_NAME>"), packageLibName)
        |> Str.global_replace(
             r("<PACKAGE_NAME_UPPER_CAMEL>"),
             packageNameUpperCamelCase,
           );
      write("package.json", packageJSON);
    };

    let appReDir = Path.(projectPath / "executable");
    let appRePath = Path.(appReDir / packageNameUpperCamelCase ++ "App.re");

    if (!exists(appRePath)) {
      let appRe =
        Str.global_replace(
          r("<PACKAGE_NAME_UPPER_CAMEL>"),
          packageNameUpperCamelCase,
          appReTemplate,
        );
      let _ = mkdirp(appReDir);
      write(appRePath, appRe);
    };

    let utilReDir = Path.(projectPath / "library");
    let utilRePath = Path.(utilReDir / "Util.re");

    if (!exists(utilRePath)) {
      mkdirp(utilReDir);
      write(utilRePath, utilRe);
    };

    let testReDir = Path.(projectPath / "test");
    let testRePath =
      Path.(testReDir / "Test" ++ packageNameUpperCamelCase ++ ".re");

    if (!exists(testRePath)) {
      let testRe =
        Str.global_replace(
          r("<PACKAGE_NAME_UPPER_CAMEL>"),
          packageNameUpperCamelCase,
          testReTemplate,
        );
      let _ = mkdirp(testReDir);
      write(testRePath, testRe);
    };

    let readMePath = Path.(projectPath / "README.md");

    if (!exists(readMePath)) {
      let readMe =
        readMeTemplate
        |> Str.global_replace(r("<PACKAGE_NAME_FULL>"), packageNameKebab)
        |> Str.global_replace(r("<PACKAGE_NAME>"), packageNameKebab)
        |> Str.global_replace(r("<PUBLIC_LIB_NAME>"), packageLibName)
        |> Str.global_replace(
             r("<PACKAGE_NAME_UPPER_CAMEL>"),
             packageNameUpperCamelCase,
           );
      write(readMePath, readMe);
    };

    let gitignorePath = Path.(projectPath / ".gitignore");

    if (!exists(gitignorePath)) {
      let gitignore =
        gitignoreTemplate
        |> Str.global_replace(r("<PACKAGE_NAME>"), packageNameKebab)
        |> Str.global_replace(r("<PUBLIC_LIB_NAME>"), packageLibName)
        |> Str.global_replace(
             r("<PACKAGE_NAME_UPPER_CAMEL>"),
             packageNameUpperCamelCase,
           );
      write(".gitignore", gitignore);
    };

    let libKebab = packageNameKebabSansScope;
    let duneProjectFile = Path.(projectPath / "dune-project");
    if (!exists(duneProjectFile)) {
      write(duneProjectFile, spf({|(lang dune 1.2)
(name %s)
|}, libKebab));
      fprintf(stdout, "    ☒  Generated dune-project");
    };

    let opamFileName = libKebab ++ ".opam";
    let opamFile = Path.(projectPath / opamFileName);
    if (!exists(opamFile)) {
      write(opamFile, "");
      fprintf(stdout, "    ☒  Generated %s", opamFileName);
    };

    let rootDuneFile = Path.(projectPath / "dune");

    if (!exists(rootDuneFile)) {
      write(rootDuneFile, "(ignored_subdirs (node_modules))");
      fprintf(stdout, "    ☒  Generated dune (root)");
    };
  };

let generateBuildFiles = projectRoot => {
  let packageJSONPath = Path.(projectRoot / "package.json");
  PesyConf.gen(projectRoot, packageJSONPath);
};
