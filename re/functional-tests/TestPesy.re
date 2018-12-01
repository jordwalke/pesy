/* TODO: Exit status of the System commands are not handled propertly. Test failures are not reported properly. Needs work */

let rec rimraf = path => {
  let isDirectory =
    try (Sys.is_directory(path)) {
    | _ => false
    };
  Sys.(
    if (isDirectory) {
      let files = readdir(path);
      Array.iter(f => rimraf(Filename.concat(path, f)), files);
      /* Unix.rmdir(path); */
    } else if (file_exists(path)) {
      remove(path);
    } else {
      ();
    }
  );
};

let tmpDir = Filename.get_temp_dir_name();
let testProject = "test-project";
let testProjectDir = Filename.concat(tmpDir, testProject);

module Path = {
  let (/) = Filename.concat;
};

let copyTemplate = tpl => {
  let _ =
    Sys.command(
      "cp "
      ++ Path.(
           (
             switch (Sys.getenv_opt("PWD")) {
             | Some(x) => x
             | None => "something-went-wrong"
             }
           )
           / tpl
         )
      ++ " "
      ++ Path.(testProjectDir / "share" / "template-repo"),
    );
  ();
  /* print_endline("Copied " ++ tpl); */
};

rimraf(testProjectDir);
Sys.command("mkdir " ++ testProjectDir);
Sys.command("mkdir " ++ Path.(testProjectDir / "bin"));
Sys.command("mkdir " ++ Path.(testProjectDir / "share"));
Sys.command("mkdir " ++ Path.(testProjectDir / "share" / "template-repo"));
Sys.command(
  "cp "
  ++ Path.(
       (
         switch (Sys.getenv_opt("PWD")) {
         | Some(x) => x
         | None => "something-went-wrong"
         }
       )
       / "_build"
       / "default"
       / "executable"
       / "Pesy.exe"
     )
  ++ " "
  ++ Path.(testProjectDir / "bin" / "pesy"),
);
copyTemplate("pesy-package.template.json");
copyTemplate("pesy-App.template.re");
copyTemplate("pesy-Test.template.re");
copyTemplate("pesy-Util.template.re");
copyTemplate("pesy-README.template.md");
copyTemplate("pesy-gitignore.template");

Sys.chdir(testProjectDir);

Unix.putenv(
  "PATH",
  Path.(testProjectDir / "bin")
  ++ (Sys.unix ? ":" : ";")
  ++ Sys.getenv("PATH"),
);

Sys.command("pesy");
Sys.command("pesy build");
Sys.command("esy b dune exec TestProjectApp.exe");
Sys.command("esy b dune exec TestTestProject.exe");
