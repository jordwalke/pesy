let prompt = q => {
  print_endline(q);
  read_line();
};

let kebab = str => {
  let charStrings = Str.split(Str.regexp_string(""), str);
  let k =
    String.concat(
      "",
      List.map(
        c =>
          if (Char.code(c.[0]) >= 65 && Char.code(c.[0]) <= 90) {
            "-" ++ String.lowercase_ascii(c);
          } else {
            c;
          },
        charStrings,
      ),
    );
  if (k.[0] == '-') {
    String.sub(k, 1, String.length(k) - 1);
  } else {
    k;
  };
};
/* TODO write inline unit tests */

let getCurrentDirName = () => Filename.basename(Sys.getenv("PWD"));

let upperCamelCasify = kebab => {
  let parts = Str.split(Str.regexp_string("-"), kebab);
  let k = String.concat("", List.map(String.capitalize_ascii, parts));
  if (k.[0] == '-') {
    String.sub(k, 1, String.length(k) - 1);
  } else {
    k;
  };
};

let removeScope = kebab =>
  Str.global_replace(Str.regexp("[^\\/]*/"), "", kebab);

let getEnv = Sys.getenv_opt;

let write = (file, str) => {
  let fileChannel =
    open_out_gen([Open_creat, Open_text, Open_append], 0o640, file);
  Printf.fprintf(fileChannel, "%s", str);
  close_out(fileChannel);
};

let readFile = file => {
  let buf = ref("");
  let breakOut = ref(false);
  let ic = open_in(file);
  while (! breakOut^) {
    let line =
      try (input_line(ic)) {
      | End_of_file => ""
      };
    if (line == "") {
      breakOut := true;
    } else {
      buf := buf^ ++ "\n" ++ line;
    };
  };
  buf^;
};

module Path = {
  let (/) = Filename.concat;
};

let parent = Filename.dirname;

let loadTemplate = name =>
  readFile(
    Path.(
      (Sys.executable_name |> parent |> parent)
      / "share"
      / "template-repo"
      / name
    ),
  );

let r = Str.regexp;

let exists = Sys.file_exists;

let mkdirp = dirs => {
  let _ =
    Sys.command(
      "mkdir -p " ++ List.fold_left((acc, e) => Path.(acc / e), "", dirs),
    );
  ();
};
