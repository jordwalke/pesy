open Printf;

let userCommand =
  if (Array.length(Sys.argv) > 1) {
    Some(Sys.argv[1]);
  } else {
    None;
  };

/* TODO: prompt user for their choice */
let projectRoot = Sys.getcwd();

/* use readFileOpt to read previously computed directory path */
PesyLib.bootstrapIfNecessary(projectRoot);
PesyLib.generateBuildFiles(projectRoot);

/*  @esy-ocaml/foo-package -> foo-package */
print_endline("Installing deps and building...");

print_endline("Running `esy install`");
let setupStatus = Sys.command("esy i");
if (setupStatus != 0) {
  fprintf(stderr, "esy install failed!");
};

print_endline("Running `esy build`");
let setupStatus = Sys.command("esy b");
if (setupStatus != 0) {
  fprintf(stderr, "esy build failed!");
};
