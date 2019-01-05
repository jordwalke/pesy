open Printf;
open PesyUtils;
open PesyUtils.NoLwt;

type common = {
  path: string,
  name: string,
  require: list(string),
};
type executable = {main: string};
type library = {namespace: string};
type pkgType =
  | Executable(executable)
  | Library(library);
type package = {
  common,
  pkgType,
};

exception GenericException(string);

let getSuffix = name => {
  let parts = Str.split(Str.regexp("\\."), name);
  switch (List.rev(parts)) {
  | [_]
  | [] =>
    raise(
      GenericException(
        "`name` property of the package must either be <package>.exe (for executables) or <package>.<suffix> for libraries, where of course suffix is not exe for libraries",
      ),
    )
  | [suffix, ...r] => suffix
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
let%expect_test _ = {
  print_endline(getSuffix("foo.bar.lib"));
  %expect
  {|
     lib
     |};
};

let%expect_test _ = {
  print_endline(
    try (getSuffix("foo")) {
    | GenericException(_) => "Must throw GenericException"
    | _ => "Did not throw GenericException"
    },
  );
  %expect
  {|
     Must throw GenericException
     |};
};

module FieldTypes = {
  type t =
    | String(string)
    | List(list(t));
  exception ConversionException(string);
  let toString =
    fun
    | String(s) => s
    | List(_) => raise(ConversionException("Expected string. Actual list"));
  let toList =
    fun
    | List(l) => l
    | String(_) =>
      raise(ConversionException("Expected list. Actual string"));
};

module JSON: {
  type t;
  let fromFile: string => t;
  let member: (t, string) => t;
  let toKeyValuePairs: t => list((string, t));
  let toValue: t => FieldTypes.t;
  let debug: t => string;
} = {
  open Yojson.Basic;
  type t = json;
  exception InvalidJSONValue(string);
  exception MissingJSONMember(string);
  let fromFile = path => from_file(path);
  let member = (j, m) =>
    try (Util.member(m, j)) {
    | _ =>
      raise(
        MissingJSONMember(Printf.sprintf("%s was missing in the json", m)),
      )
    };
  let toKeyValuePairs = (json: json) =>
    switch (json) {
    | `Assoc(jsonKeyValuePairs) => jsonKeyValuePairs
    | _ => raise(InvalidJSONValue("Expected key value pairs"))
    };
  let rec toValue = (json: json) =>
    switch (json) {
    | `String(s) => FieldTypes.String(s)
    | `List(jl) => FieldTypes.List(List.map(j => toValue(j), jl))
    | _ =>
      raise(
        InvalidJSONValue(
          sprintf(
            "Value must be either list of string. Found %s",
            to_string(json),
          ),
        ),
      )
    };
  let debug = t => to_string(t);
};

type t = list(package);
let toPesyConf = (projectPath: string, json: JSON.t): t => {
  let pkgs = JSON.toKeyValuePairs(JSON.member(json, "buildDirs"));
  List.map(
    pkg => {
      let (dir, conf) = pkg;
      let name =
        JSON.member(conf, "name") |> JSON.toValue |> FieldTypes.toString;
      let require =
        try (
          JSON.member(conf, "require")
          |> JSON.toValue
          |> FieldTypes.toList
          |> List.map(FieldTypes.toString)
        ) {
        | _ => []
        };

      let suffix = getSuffix(name);

      switch (suffix) {
      | "exe" =>
        let main =
          JSON.member(conf, "main") |> JSON.toValue |> FieldTypes.toString;
        {
          common: {
            name,
            path: Path.(projectPath / dir / ""),
            require,
          },
          pkgType: Executable({main: main}),
        };
      | _ =>
        let namespace =
          JSON.member(conf, "namespace")
          |> JSON.toValue
          |> FieldTypes.toString;
        {
          common: {
            name,
            path: Path.(projectPath / dir / ""),
            require,
          },
          pkgType: Library({namespace: namespace}),
        };
      };
    },
    pkgs,
  );
};

module Stanza: {
  type t;
  let create: (string, t) => t;
  let createAtom: string => t;
  let createExpression: list(t) => t;
  let toSexp: t => Sexplib.Sexp.t;
} = {
  open Sexplib.Sexp;
  type t = Sexplib.Sexp.t;
  let createAtom = a => Atom(a);
  let create = (stanza: string, expression) =>
    List([Atom(stanza), expression]);
  let createExpression = atoms => List(atoms);
  let toSexp = x => x;
};

module DuneFile: {let toString: list(Stanza.t) => string;} = {
  open Sexplib.Sexp;
  let toString = (stanzas: list(Stanza.t)) =>
    List.fold_right(
      (s, acc) => to_string_hum(~indent=4, Stanza.toSexp(s)) ++ acc,
      stanzas,
      "",
    );
};
let toPackages = (_prjPath, pkgs) =>
  List.iter(
    pkg => {
      let {name: pkgName, path, require} = pkg.common;
      switch (pkg.pkgType) {
      | Library({namespace}) =>
        let name = Stanza.create("name", Stanza.createAtom(pkgName));
        let public_name =
          Stanza.create("public_name", Stanza.createAtom(pkgName));
        let libraries =
          switch (require) {
          | [] => None
          | libs =>
            Some(
              Stanza.createExpression([
                Stanza.createAtom("libraries"),
                ...List.map(r => Stanza.createAtom(r), libs),
              ]),
            )
          };

        let library =
          Stanza.createExpression(
            [Stanza.createAtom("library"), name, public_name]
            @ (
              switch (libraries) {
              | None => []
              | Some(x) => [x]
              }
            ),
          );

        printf(
          "Wrote (%s) \n %s\n",
          Path.(path / "dune"),
          DuneFile.toString([library]),
        );

        mkdirp(path);
        write(Path.(path / "dune"), DuneFile.toString([library]));

      | Executable({main}) =>
        let name = Stanza.create("name", Stanza.createAtom(main));
        let public_name =
          Stanza.create("public_name", Stanza.createAtom(pkgName));
        let libraries =
          switch (require) {
          | [] => None
          | libs =>
            Some(
              Stanza.createExpression([
                Stanza.createAtom("libraries"),
                ...List.map(r => Stanza.createAtom(r), libs),
              ]),
            )
          };

        let executable =
          Stanza.createExpression(
            [Stanza.createAtom("executable"), name, public_name]
            @ (
              switch (libraries) {
              | None => []
              | Some(x) => [x]
              }
            ),
          );

        printf(
          "Wrote (%s) \n %s\n",
          Path.(path / "dune"),
          DuneFile.toString([executable]),
        );

        mkdirp(path);
        write(Path.(path / "dune"), DuneFile.toString([executable]));
      };
    },
    pkgs,
  );

let gen = (prjPath, pkgPath) => {
  let json = JSON.fromFile(pkgPath);
  let packages = toPesyConf(prjPath, json);
  toPackages(prjPath, packages);
};
