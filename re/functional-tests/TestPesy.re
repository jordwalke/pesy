/* TODO: Exit status of the System commands are not handled propertly. Test failures are not reported properly. Needs work */

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

Sys.command("rm -rf " ++ testProjectDir);
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
  ++ Path.(testProjectDir / "bin"),
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
  Sys.getenv("PATH")
  ++ (Sys.unix ? ":" : ";")
  ++ Path.(testProjectDir / "bin"),
);

Sys.command("Pesy.exe --test-mode");
Sys.command("esy pesy");
exit(Sys.command("esy x TestProjectApp.exe"));
