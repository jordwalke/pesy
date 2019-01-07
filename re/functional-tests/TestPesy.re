let rimraf = s =>
  switch (Bos.OS.Dir.delete(~recurse=true, Fpath.v(s))) {
  | Ok () => ()
  | _ => Printf.fprintf(stdout, "Warning: Could not delete %s\n", s)
  };

let buffer_size = 8192;
let buffer = Bytes.create(buffer_size);

let command_output = command => {
  let c = Bos.Cmd.v(command);
  switch (Bos.OS.Cmd.(run_out(c) |> out_string)) {
  | Ok((output, _return_code)) => output
  | _ =>
    Printf.fprintf(stderr, "`%s` failed.", command);
    exit(-1);
  };
};

let mkdir = (~perms=?, p) =>
  switch (perms) {
  | Some(x) => Unix.mkdir(p, x)
  | None => Unix.mkdir(p, 0o755)
  };

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

module Path = {
  let (/) = Filename.concat;
};

let parent = Filename.dirname;

let tmpDir = Filename.get_temp_dir_name();
let testProject = "test-project";
let testProjectDir = Filename.concat(tmpDir, testProject);
let pesyBinPath =
  Path.(
    testProjectDir / "_build" / "install" / "default" / "bin" / "Pesy.exe"
  );

rimraf(testProjectDir); /* So that we can run it stateless locally */
mkdir(testProjectDir);
Sys.chdir(testProjectDir);

if (Sys.command(pesyBinPath) != 0) {
  Printf.fprintf(
    stderr,
    "Test failed: Non zero exit when running bootstrapper",
  );
  exit(-1);
};

let exitStatus = Sys.command("esy x TestProjectApp.exe");

if (exitStatus != 0) {
  Printf.fprintf(
    stderr,
    "Test failed: Non zero exit when running TestProjectApp.exe",
  );
  exit(-1);
};

let exitStatus = Sys.command("esy x TestTestProject.exe");
if (exitStatus != 0) {
  Printf.fprintf(
    stderr,
    "Test failed: Non zero exit when running TestTestProject.exe",
  );
  exit(-1);
};
