/* TODO: Exit status of the System commands are not handled propertly. Test failures are not reported properly. Needs work */

let rimraf = s => {
  let _ = Bos.OS.Dir.delete(Fpath.v(s));
};


let buffer_size = 8192;
let buffer = Bytes.create(buffer_size);

let file_copy = (input_name, output_name) => {
  open Unix;
  let fd_in = openfile(input_name, [O_RDONLY], 0);
  let fd_out = openfile(output_name, [O_WRONLY, O_CREAT, O_TRUNC], 438);
  let rec copy_loop = () =>
    switch (read(fd_in, buffer, 0, buffer_size)) {
    | 0 => ()
    | r =>
      ignore(write(fd_out, buffer, 0, r));
      copy_loop();
    };

  copy_loop();
  close(fd_in);
  close(fd_out);
};


let tmpDir = Filename.get_temp_dir_name();
let testProject = "test-project";
let testProjectDir = Filename.concat(tmpDir, testProject);

module Path = {
  let (/) = Filename.concat;
};

let copyTemplate = tpl => {
  file_copy(
    Path.(
         Sys.getcwd()
          / tpl
        )
    ,
    Path.(testProjectDir / "share" / "template-repo" / tpl),
  );
  /* print_endline("Copied " ++ tpl); */
};

rimraf(testProjectDir);
Sys.command("mkdir " ++ testProjectDir);
Sys.command("mkdir " ++ Path.(testProjectDir / "bin"));
Sys.command("mkdir " ++ Path.(testProjectDir / "share"));
Sys.command("mkdir " ++ Path.(testProjectDir / "share" / "template-repo"));
file_copy(
  Path.(
       Sys.getcwd()
       / "_build"
       / "default"
       / "executable"
       / "Pesy.exe"
     )
,
  Path.(testProjectDir / "bin" / "pesy.exe"),
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

Sys.command("pesy.exe");
/* Sys.command("pesy.exe build"); */
Sys.command("esy b dune build");
let pid = try (Unix.create_process(Path.(Sys.getcwd() / "_build" / "default" / "executable" / "TestProjectApp.exe"), [||], Unix.stdin, Unix.stdout, Unix.stderr)) {
  | Unix.Unix_error(e, _, _) => {
      print_endline(Unix.error_message(e));
      exit(-1);
    }
}
let exitStatus = switch (Unix.waitpid([], pid)) {
  | (_, WEXITED(c)) => c
  | (_, WSIGNALED(c)) => c
  | (_, WSTOPPED(c)) => c
  };
  if (exitStatus != 0) {
    print_endline("Test failed: Non zero exit when running TestProjectApp.exe");
  }
  let pid = Unix.create_process(Path.(Sys.getcwd() / "_build" / "default" / "test" / "TestTestProject.exe"), [||], Unix.stdin, Unix.stdout, Unix.stderr);
let exitStatus = switch (Unix.waitpid([], pid)) {
  | (_, WEXITED(c)) => c
  | (_, WSIGNALED(c)) => c
  | (_, WSTOPPED(c)) => c
  }; 
  if (exitStatus != 0) {
    print_endline("Test failed: Non zero exit when running TestTestProject.exe");
  }
