open PesyUtils;

let userCommand =
  if (Array.length(Sys.argv) > 1) {
    Some(Sys.argv[1]);
  } else {
    None;
  };

let projectRoot =
  switch (Sys.getenv_opt("cur__root")) {
  | Some(curRoot) =>
    /**
     * This means the user ran pesy in an esy environment.
     * Either as
     * 1. esy pesy
     * 2. esy b pesy
     * 3. esy pesy build
     * 4. esy b pesy build
     */
    curRoot
  | None =>
    /**
     * This mean pesy is being run naked on the shell.
     * Either it was:
     *    $ pesy
     *    $ pesy build
     */
    /** TODO prompt user for custom path */
    Sys.getcwd()
  };

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
    LTerm.printls(LTerm_text.eval([LTerm_text.S("esy install failed!")]));
  } else {
    Lwt.return();
  };
