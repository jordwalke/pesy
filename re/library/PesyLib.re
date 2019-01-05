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

let bootstrap = testMode =>
  if (!isEsyInstalled()) {
    LTerm.printls(Copy.esyNotInstalledError);
  } else {
    print_endline(
      Printf.sprintf(
        "Create new package in the CURRENT DIRECTORY \n %s ?",
        Sys.getcwd(),
      ),
    );
    let currentDirNameKebab = kebab(getCurrentDirName());
    let packageNameAnswer =
      !testMode ?
        prompt(
          Printf.sprintf(
            "Enter package name (lowercase/hyphens) [default %s]:",
            currentDirNameKebab,
          ),
        ) :
        currentDirNameKebab;
    let packageNameKebab =
      packageNameAnswer == "" ?
        currentDirNameKebab : kebab(packageNameAnswer);
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
    let packageJSON =
      packageJSONTemplate
      |> Str.global_replace(r("<PACKAGE_NAME_FULL>"), packageNameKebab)
      |> Str.global_replace(r("<VERSION>"), version)
      |> Str.global_replace(r("<PUBLIC_LIB_NAME>"), packageLibName)
      |> Str.global_replace(
           r("<PACKAGE_NAME_UPPER_CAMEL>"),
           packageNameUpperCamelCase,
         );

    let appRe =
      Str.global_replace(
        r("<PACKAGE_NAME_UPPER_CAMEL>"),
        packageNameUpperCamelCase,
        appReTemplate,
      );

    let testRe =
      Str.global_replace(
        r("<PACKAGE_NAME_UPPER_CAMEL>"),
        packageNameUpperCamelCase,
        testReTemplate,
      );

    let readMe =
      readMeTemplate
      |> Str.global_replace(r("<PACKAGE_NAME_FULL>"), packageNameKebab)
      |> Str.global_replace(r("<PACKAGE_NAME>"), packageNameKebab)
      |> Str.global_replace(r("<PUBLIC_LIB_NAME>"), packageLibName)
      |> Str.global_replace(
           r("<PACKAGE_NAME_UPPER_CAMEL>"),
           packageNameUpperCamelCase,
         );

    let gitignore =
      gitignoreTemplate
      |> Str.global_replace(r("<PACKAGE_NAME>"), packageNameKebab)
      |> Str.global_replace(r("<PUBLIC_LIB_NAME>"), packageLibName)
      |> Str.global_replace(
           r("<PACKAGE_NAME_UPPER_CAMEL>"),
           packageNameUpperCamelCase,
         );

    let%lwt _ =
      if (exists("package.json")) {
        LTerm.printls(Copy.projectExistsError);
      } else {
        write("package.json", packageJSON);
      };

    let appRePath = Path.(Sys.getcwd() / "executable");
    let _ = mkdirp(appRePath);
    let%lwt _ =
      write(Path.(appRePath / packageNameUpperCamelCase ++ "App.re"), appRe);
    let utilRePath = Path.(Sys.getcwd() / "library");
    let _ = mkdirp(utilRePath);
    let%lwt _ = write(Path.(utilRePath / "Util.re"), utilRe);
    let testRePath = Path.(Sys.getcwd() / "test");
    let _ = mkdirp(testRePath);
    let%lwt _ =
      write(
        Path.(testRePath / "Test" ++ packageNameUpperCamelCase ++ ".re"),
        testRe,
      );

    let%lwt _ =
      if (exists("README.md")) {
        LTerm.printls(Copy.readmeExistsWarning);
      } else {
        write("README.md", readMe);
      };

    let%lwt _ =
      if (exists(".gitignore")) {
        LTerm.printls(Copy.gitignoreExistsWarning);
      } else {
        write(".gitignore", gitignore);
      };

    /*  @esy-ocaml/foo-package -> foo-package */
    let%lwt _ =
      LTerm.printls(
        LTerm_text.eval([
          LTerm_text.S(
            "package.json created. Running 'esy install' and 'esy pesy'\\n\\n",
          ),
          LTerm_text.S(packageNameKebab ++ "@" ++ version ++ "\n"),
        ]),
      );

    let setupStatus = Sys.command("esy i");
    if (setupStatus != 0) {
      LTerm.printls(LTerm_text.eval([LTerm_text.S("Error!")]));
    } else {
      LTerm.printls(LTerm_text.eval([LTerm_text.S("Done")]));
    };
  };

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

    /*  @esy-ocaml/foo-package -> foo-package */
    let%lwt _ =
      LTerm.printls(
        LTerm_text.eval([LTerm_text.S("Installing deps and building...")]),
      );

    let%lwt setupStatus = Lwt.return(Sys.command("esy i"));
    if (setupStatus != 0) {
      LTerm.printls(LTerm_text.eval([LTerm_text.S("Error!")]));
    } else {
      LTerm.printls(LTerm_text.eval([LTerm_text.S("Done")]));
    };
  };

let generateBuildFiles = (projectRoot, pesyConf: Conf.t) => {
  let testMainModule = pesyConf.test.main; /* TODO: Deal with mising .re */
  let testMainModuleName = pesyConf.test.main;
  let testBin = pesyConf.test.name;
  let testRequire = pesyConf.test.require;
  let testBinDir = Path.(projectRoot / "test");
  let testBinDuneFile = Path.(testBinDir / "dune");

  let libName = pesyConf.library.name;
  let libNamespace = pesyConf.library.namespace;
  let libRequire = pesyConf.library.require;
  let libraryDir = Path.(projectRoot / "library");
  let libraryDuneFile = Path.(libraryDir / "dune");

  let binMain = pesyConf.executable.main;
  let binName = pesyConf.executable.name;
  let binRequire = pesyConf.executable.require;
  let executableDir = Path.(projectRoot / "executable");
  let executableDuneFile = Path.(executableDir / "dune");

  let%lwt _ =
    LTerm.printls(
      LTerm_text.of_string(
        renderAsciiTree(
          "test",
          spf("name:    %s", testMainModuleName),
          spf("main:    %s", testMainModule),
          spf(
            "require: %s",
            List.fold_left((acc, e) => acc ++ " " ++ e, "", testRequire),
          ),
          false,
        ),
      ),
    );

  let%lwt _ =
    LTerm.printls(
      LTerm_text.of_string(
        renderAsciiTree(
          "library",
          spf("name:      %s", libName),
          spf("namespace: %s", libNamespace),
          spf(
            "require:   %s",
            List.fold_left((acc, e) => acc ++ " " ++ e, "", libRequire),
          ),
          false,
        ),
      ),
    );

  let%lwt _ =
    LTerm.printls(
      LTerm_text.of_string(
        renderAsciiTree(
          "executable",
          spf("name:      %s", binName),
          spf("main:      %s", binMain),
          spf(
            "require:   %s",
            List.fold_left((acc, e) => acc ++ " " ++ e, "", binRequire),
          ),
          true,
        ),
      ),
    );

  let testDuneFileContents =
    switch (readFileOpt(testBinDuneFile)) {
    | Some(x) => x
    | None => "()"
    };

  let existingTestBinDuneSexp = Sexplib.Sexp.of_string(testDuneFileContents);

  let libraryAtoms = List.map(e => Sexplib.Sexp.Atom(e), testRequire);
  let newTestBinDuneSexp =
    Sexplib.Sexp.(
      List(
        [
          Atom("executable"),
          List([Atom("name"), Atom(testMainModuleName)]),
          List([Atom("public_name"), Atom(testBin)]),
        ]
        @ (
          libraryAtoms == [] ?
            [] : [List([Atom("libraries"), ...libraryAtoms])]
        ),
      )
    );

  let%lwt _ =
    Sexplib.Sexp.(
      switch (existingTestBinDuneSexp) {
      | Atom(x) =>
        LTerm.printls(
          LTerm_text.of_string(
            spf("Malformed dune file? Only %s was found", x),
          ),
        )
      | List(x) =>
        let%lwt _ =
          write(
            testBinDuneFile,
            Sexplib.Sexp.to_string_hum(~indent=4, newTestBinDuneSexp),
          );
        if (List.length(x) == 0) {
          LTerm.printls(
            LTerm_text.of_string(
              "    ☒  Generated test executable/dune build config",
            ),
          );
        } else if (newTestBinDuneSexp != existingTestBinDuneSexp) {
          LTerm.printls(
            LTerm_text.of_string(
              "    ☒  Updated test executable/dune build config",
            ),
          );
        } else {
          Lwt.return();
        };
      }
    );

  let libraryDuneFileContents =
    switch (readFileOpt(libraryDuneFile)) {
    | Some(x) => x
    | None => "()"
    };

  let existingLibraryDuneSexp =
    Sexplib.Sexp.of_string(libraryDuneFileContents);

  let libraryRequireAtoms = List.map(e => Sexplib.Sexp.Atom(e), libRequire);
  let newlibraryDuneSexp =
    Sexplib.Sexp.(
      List(
        [
          Atom("library"),
          List([Atom("name"), Atom(libNamespace)]),
          List([Atom("public_name"), Atom(libName)]),
        ]
        @ (
          libraryRequireAtoms == [] ?
            [] : [List([Atom("libraries"), ...libraryRequireAtoms])]
        ),
      )
    );

  let%lwt _ =
    Sexplib.Sexp.(
      switch (existingLibraryDuneSexp) {
      | Atom(x) =>
        LTerm.printls(
          LTerm_text.of_string(
            spf("Malformed dune file? Only %s was found", x),
          ),
        )
      | List(x) =>
        let%lwt _ =
          write(
            libraryDuneFile,
            Sexplib.Sexp.to_string_hum(~indent=4, newlibraryDuneSexp),
          );
        if (List.length(x) == 0) {
          LTerm.printls(
            LTerm_text.of_string(
              "    ☒  Generated test library/dune build config",
            ),
          );
        } else if (newlibraryDuneSexp != existingLibraryDuneSexp) {
          LTerm.printls(
            LTerm_text.of_string(
              "    ☒  Updated library/dune build config",
            ),
          );
        } else {
          Lwt.return();
        };
      }
    );

  let executableDuneFileContents =
    switch (readFileOpt(executableDuneFile)) {
    | Some(x) => x
    | None => "()"
    };

  let existingExecutableDuneSexp =
    Sexplib.Sexp.of_string(executableDuneFileContents);

  let executableRequireAtoms =
    List.map(e => Sexplib.Sexp.Atom(e), binRequire);

  let newExecutableDuneSexp =
    Sexplib.Sexp.(
      List(
        [
          Atom("executable"),
          List([Atom("name"), Atom(binMain)]),
          List([Atom("public_name"), Atom(binName)]),
        ]
        @ (
          executableRequireAtoms == [] ?
            [] : [List([Atom("libraries"), ...executableRequireAtoms])]
        ),
      )
    );

  let%lwt _ =
    Sexplib.Sexp.(
      switch (existingExecutableDuneSexp) {
      | Atom(x) =>
        LTerm.printls(
          LTerm_text.of_string(
            spf("Malformed dune file? Only %s was found", x),
          ),
        )
      | List(x) =>
        let%lwt _ =
          write(
            executableDuneFile,
            Sexplib.Sexp.to_string_hum(~indent=4, newExecutableDuneSexp),
          );
        if (List.length(x) == 0) {
          LTerm.printls(
            LTerm_text.of_string(
              "    ☒  Generated test executable/dune build config",
            ),
          );
        } else if (newExecutableDuneSexp != existingExecutableDuneSexp) {
          LTerm.printls(
            LTerm_text.of_string(
              "    ☒  Updated executable/dune build config",
            ),
          );
        } else {
          Lwt.return();
        };
      }
    );

  let libKebab = kebab(libNamespace);
  let duneProjectFile = Path.(projectRoot / "dune-project");
  let%lwt _ =
    if (!exists(duneProjectFile)) {
      let%lwt _ =
        write(
          duneProjectFile,
          spf({|(lang dune 1.2)
(name %s)
|}, libKebab),
        );
      LTerm.printls(LTerm_text.of_string("    ☒  Generated dune-project"));
    } else {
      Lwt.return();
    };

  let opamFileName = libKebab ++ ".opam";
  let opamFile = Path.(projectRoot / opamFileName);
  let%lwt _ =
    if (!exists(opamFile)) {
      let%lwt _ = write(opamFile, "");
      LTerm.printls(
        LTerm_text.of_string(spf("    ☒  Generated %s", opamFileName)),
      );
    } else {
      Lwt.return();
    };

  let rootDuneFile = Path.(projectRoot / "dune");
  if (!exists(rootDuneFile)) {
    let%lwt _ = write(rootDuneFile, "(ignored_subdirs (node_modules))");
    LTerm.printls(LTerm_text.of_string("    ☒  Generated dune (root)"));
  } else {
    Lwt.return();
  };
};

module PesyConf = PesyConf;
