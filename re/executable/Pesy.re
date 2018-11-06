open PesyUtils;

let mode: PesyLib.Mode.EsyEnv.t' =
  PesyLib.Mode.EsyEnv.(
    switch (Sys.getenv_opt("cur__root")) {
    | Some(curRoot) =>
      switch (Sys.getenv_opt("SHELL")) {
      | Some(shell) =>
        Str.search_forward(Str.regexp("noprofile"), shell, 0) != (-1) ?
          ESY_ENV(BUILD, curRoot) : ESY_ENV(SHELL, curRoot)
      | None =>
        print_endline("Warning: Weird. cur__name present but no SHELL");
        ESY_ENV(SHELL, curRoot);
      }
    | None => NAKED
    }
  );

let mode =
  PesyLib.Mode.EsyEnv.ESY_ENV(PesyLib.Mode.EsyEnv.SHELL, Sys.getenv("PWD"));

PesyLib.Mode.EsyEnv.(
  Lwt_main.run(
    switch (mode) {
    | ESY_ENV(x, curRoot) =>
      let packageJSONPath = Path.(curRoot / "package.json");
      let buildDirs = PesyLib.extractPesyConf(packageJSONPath);
      PesyLib.genBuildFiles(x, curRoot, buildDirs);

    | NAKED =>
      PesyLib.bootstrap(
        Array.length(Sys.argv) > 1 && Sys.argv[1] == "--test-mode",
      )
    },
  )
);
