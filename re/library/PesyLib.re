open PesyUtils;
open Utils;
module Mode = Mode;
module PesyConf = PesyConf;

let bootstrapIfNecessary = projectPath =>
  if (!isEsyInstalled()) {
    LTerm.printls(Copy.esyNotInstalledError);
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

    let%lwt _ =
      if (exists("package.json")) {
        Lwt.return();
      } else {
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
    let%lwt _ =
      if (exists(appRePath)) {
        Lwt.return();
      } else {
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
    let%lwt _ =
      if (exists(utilRePath)) {
        Lwt.return();
      } else {
        let _ = mkdirp(utilReDir);
        write(utilRePath, utilRe);
      };

    let testReDir = Path.(projectPath / "test");
    let testRePath =
      Path.(testReDir / "Test" ++ packageNameUpperCamelCase ++ ".re");
    let%lwt _ =
      if (exists(testRePath)) {
        Lwt.return();
      } else {
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
    let%lwt _ =
      if (exists(readMePath)) {
        Lwt.return();
      } else {
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
    let%lwt _ =
      if (exists(gitignorePath)) {
        Lwt.return();
      } else {
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
    let%lwt _ =
      if (!exists(duneProjectFile)) {
        let%lwt _ =
          write(
            duneProjectFile,
            spf({|(lang dune 1.2)
(name %s)
|}, libKebab),
          );
        LTerm.printls(
          LTerm_text.of_string("    ☒  Generated dune-project"),
        );
      } else {
        Lwt.return();
      };

    let opamFileName = libKebab ++ ".opam";
    let opamFile = Path.(projectPath / opamFileName);
    let%lwt _ =
      if (!exists(opamFile)) {
        let%lwt _ = write(opamFile, "");
        LTerm.printls(
          LTerm_text.of_string(spf("    ☒  Generated %s", opamFileName)),
        );
      } else {
        Lwt.return();
      };

    let rootDuneFile = Path.(projectPath / "dune");

    if (!exists(rootDuneFile)) {
      let%lwt _ = write(rootDuneFile, "(ignored_subdirs (node_modules))");
      LTerm.printls(LTerm_text.of_string("    ☒  Generated dune (root)"));
    } else {
      Lwt.return();
    };
  };

let generateBuildFiles = projectRoot => {
  let packageJSONPath = Path.(projectRoot / "package.json");
  Lwt.return(PesyConf.gen(projectRoot, packageJSONPath));
};
