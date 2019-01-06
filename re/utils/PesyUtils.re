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

let write = (file, str) => {
  open Lwt_io;
  let%lwt fileChannel = open_file(Output, file);
  let%lwt _ = write_line(fileChannel, str);
  close(fileChannel);
};

module NoLwt = {
  open Printf;
  let write = (file, str) => {
    let oc = open_out(file);
    fprintf(oc, "%s", str);
    close_out(oc);
  };
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

let mkdir = (~perms=?, p) =>
  switch (perms) {
  | Some(x) => Unix.mkdir(p, x)
  | None => Unix.mkdir(p, 0o755)
  };

let mkdirp = p => {
  let directory_created =
    try (Sys.is_directory(p)) {
    | Sys_error(_) => false
    };
  if (!directory_created) {
    mkdir(p);
  };
};

let spf = Printf.sprintf;

let renderAsciiTree = (dir, name, namespace, require, isLast) =>
  if (isLast) {
    spf({js|│
└─ %s
   %s
   %s
|js}, dir, name, namespace);
  } else {
    Printf.sprintf(
      {js|│
├─ %s
│  %s
│  %s
|js},
      dir,
      name,
      namespace,
    )
    ++ (require == "" ? "" : (isLast ? "   " : "│  ") ++ require);
  };

let readFileOpt = f =>
  if (exists(f)) {
    Some(readFile(f));
  } else {
    None;
  };

/* module Cache = { */
/*   module CacheInternal = { */
/*     type t = {path: string}; */
/*     let init = path => {path: path}; */
/*   }; */
/*   let init = () => { */
/*     /\* let cacheStoragePath = *\/ */
/*     /\*   Path.((Sys.executable_name |> parent |> parent) / "share" / "cache"); *\/ */
/*   }; */
/*   let get = (cache: CacheInternal.t, key: string) => {}; */
/* }; */
