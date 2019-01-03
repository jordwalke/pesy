exception InvalidProperty(string, string);
exception GenericException(string);

module FieldTypes = {
  type dirName = string;
  type require = list(string);
  type name = string;
  type namespace = string;
  type main = string;
};
module Executable = {
  type t = {
    require: FieldTypes.require,
    main: FieldTypes.main,
    name: FieldTypes.name,
  };
};
module Library = {
  type t = {
    require: FieldTypes.require,
    name: FieldTypes.name,
    namespace: FieldTypes.namespace,
  };
};

module PackageConf = {
  type t =
    | Executable(Executable.t)
    | Library(Library.t);
};

type t = list(PackageConf.t);

let getSuffix = name => {
  let parts = Str.split(Str.regexp("\\."), name);
  switch (List.rev(parts)) {
  | [suffix, ..._] => suffix
  | _ =>
    raise(
      GenericException(
        "`name` property of the package must either be <package>.exe (for executables) or <package>.<suffix> for libraries, where of course suffix is not exe for libraries",
      ),
    )
  };
};

/* let%test "getSuffix(): must return suffix" = getSuffix("foo.lib") == "lib"; */

let%expect_test _ = {
  print_endline(getSuffix("foo.lib"));
  %expect
  {|
    lib
  |};
};

let toPackageConf = ((_dirName, json)) => {
  open Yojson.Basic.Util;
  let name =
    switch (member("name", json)) {
    | `String(s) => s
    | _ => raise(InvalidProperty("name", "string"))
    };

  let _suffix = getSuffix(name);
  ();
};

let extract = path => {
  open Yojson.Basic.Util;
  let packageJSON = Yojson.Basic.from_file(path); /* TODO: handle missing file */
  let pesySection = member("buildDirs", packageJSON);
  let _ =
    switch (pesySection) {
    | `Assoc(keyValuePairs) => List.map(toPackageConf, keyValuePairs)
    | _ => raise(InvalidProperty("buildDirs", "PesyConf.t"))
    };
  try (123 + 1) {
  | InvalidProperty(propertyName, expectedPropertyType) =>
    Printf.printf(
      "Invalid config for %s. Expected %s",
      propertyName,
      expectedPropertyType,
    );
    exit(-1);
  };
};
