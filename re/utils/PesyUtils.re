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
