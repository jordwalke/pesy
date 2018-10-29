open PesyUtils;
open Utils;

let build = () => LTerm.printls(LTerm_text.of_string("TODO: Build process"));

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
    let _ = Sys.command("echo '>>>>>>'; cat package.json");

    let appRePath = Path.(Sys.getcwd() / "executable");
    let _ = mkdirp(appRePath);
    let%lwt _ = write(Path.(appRePath / "App.re"), appRe);
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

    let setupStatus = Sys.command("esy i && esy b");
    if (setupStatus != 0) {
      LTerm.printls(LTerm_text.eval([LTerm_text.S("Error!")]));
    } else {
      LTerm.printls(LTerm_text.eval([LTerm_text.S("Done")]));
    };
  };
