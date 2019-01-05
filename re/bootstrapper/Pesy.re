let userCommand =
  if (Array.length(Sys.argv) > 1) {
    Some(Sys.argv[1]);
  } else {
    None;
  };

/* TODO: prompt user for their choice */
let projectRoot = Sys.getcwd();

/* use readFileOpt to read previously computed directory path */
let%lwt _ = PesyLib.bootstrapIfNecessary(projectRoot);
let%lwt _ = PesyLib.generateBuildFiles(projectRoot);

/*  @esy-ocaml/foo-package -> foo-package */
let%lwt _ =
  LTerm.printls(
    LTerm_text.eval([LTerm_text.S("Installing deps and building...")]),
  );

let%lwt setupStatus = Lwt.return(Sys.command("esy i"));
let%lwt _ =
  if (setupStatus != 0) {
    LTerm.printls(LTerm_text.eval([LTerm_text.S("esy install failed!")]));
  } else {
    Lwt.return();
  };

let%lwt setupStatus = Lwt.return(Sys.command("esy b"));
let%lwt _ =
  if (setupStatus != 0) {
    LTerm.printls(LTerm_text.eval([LTerm_text.S("esy build failed!")]));
  } else {
    Lwt.return();
  };
