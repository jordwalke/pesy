open PesyUtils;
open Utils;

let build = () => LTerm.printls(LTerm_text.of_string("TODO: Build process"));

let bootstrap = () =>
  if (!isEsyInstalled()) {
    LTerm.printls(Copy.esyNotInstalledError);
  } else if (hasProjJson()) {
    LTerm.printls(Copy.projectExistsError);
  } else {
    print_endline(
      Printf.sprintf(
        "Create new package in the CURRENT DIRECTORY \n %s ?",
        Sys.getcwd(),
      ),
    );
    let currentDirNameKebab = kebab(getCurrentDirName());
    let packageNameAnswer =
      prompt(
        Printf.sprintf(
          "Enter package name (lowercase/hyphens) [default %s]:",
          currentDirNameKebab,
        ),
      );
    let packageNameKebab =
      packageNameAnswer == "" ?
        currentDirNameKebab : kebab(packageNameAnswer);
    let packageNameKebabSansScope = removeScope(packageNameKebab);
    let packageUpperCamelCase = upperCamelCasify(packageNameKebabSansScope);
    /*  @esy-ocaml/foo-package -> foo-package */
    LTerm.printls(
      LTerm_text.eval([
        LTerm_text.S(packageNameKebabSansScope),
        LTerm_text.S(" ====== " ++ packageUpperCamelCase),
        LTerm_text.S(" ====== " ++ packageNameKebab),
      ]),
    );
  };
