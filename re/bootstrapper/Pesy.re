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
let packageJSONPath = Path.(projectRoot / "package.json");
let buildDirs = PesyLib.extractConf(packageJSONPath);
Lwt_main.run(PesyLib.generateBuildFiles(projectRoot, buildDirs));
