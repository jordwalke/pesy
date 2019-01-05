open PesyUtils;
open Utils;

exception InvalidEnvJSON(string);

module PackageConf = {
  type t = {
    require: list(string),
    main: string,
    name: string,
  };
  module Library = {
    type t = {
      require: list(string),
      name: string,
      namespace: string,
    };
  };
};

module Conf = {
  type t = {
    test: PackageConf.t,
    library: PackageConf.Library.t,
    executable: PackageConf.t,
  };
};

let getMemberJSON = (json, m) =>
  try (Yojson.Basic.Util.(json |> member(m))) {
  | _ => Yojson.Basic.from_string("{}") /* TODO: Fixme. Silent fallback */
  };
let getMemberStr = (m, json) =>
  try (getMemberJSON(json, m) |> Yojson.Basic.Util.to_string) {
  | _ => "" /* TODO: Fixme. Silent fallback */
  };

let getNameStr = getMemberStr("name");
let getNamespaceStr = getMemberStr("namespace");
let getRequireList = json =>
  try (
    Yojson.Basic.Util.(
      json |> member("require") |> to_list |> List.map(to_string)
    )
  ) {
  | _ => [] /* TODO: Fixme. Silent fallback */
  };
let getMainStr = getMemberStr("main");

let extractConf = (p): Conf.t => {
  /* let packageJSON = Yojson.Basic.from_file(path); /\* TODO: handle missing file *\/ */

  open Yojson.Basic.Util;
  let packageJSON = Yojson.Basic.from_file(p);
  let buildDirsJSON = getMemberJSON(packageJSON, "buildDirs");
  let fromBuildDirsJSON = getMemberJSON(buildDirsJSON);
  let testJSON = fromBuildDirsJSON("test");
  let libraryJSON = fromBuildDirsJSON("library");
  let executableJSON = fromBuildDirsJSON("executable");
  Yojson.Basic.Util.{
    test: {
      require: getRequireList(testJSON),
      main: getMainStr(testJSON),
      name: getNameStr(testJSON),
    },
    library: {
      require: getRequireList(libraryJSON),
      namespace: getNamespaceStr(libraryJSON),
      name: getNameStr(libraryJSON),
    },
    executable: {
      require: getRequireList(executableJSON),
      main: getMainStr(executableJSON),
      name: getNameStr(executableJSON),
    },
  };
};

module Mode = Mode;

let build = () => {
  let _ = Sys.command("dune build");
  ();
};

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

    let libKebab = kebab(packageLibName);
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

module PesyConf = PesyConf;
