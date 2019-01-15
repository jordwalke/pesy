open Printf;
open PesyUtils;
open PesyUtils.NoLwt;

exception NullJSONValue(unit);
type common = {
  path: string,
  name: string,
  require: list(string),
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

module Library: {
  module Mode: {
    exception InvalidLibraryMode(unit);
    type t;
    let ofString: string => t;
    let toString: t => string;
  };
  type t;
  let create:
    (
      string,
      option(list(Mode.t)),
      option(list(string)),
      option(list(string)),
      option(list(string)),
      option(bool)
    ) =>
    t;
  let toDuneStanza: (common, t) => (string, string);
} = {
  module Mode = {
    exception InvalidLibraryMode(unit);
    type t =
      | Native
      | Byte
      | Best;
    let ofString =
      fun
      | "best" => Best
      | "native" => Native
      | "byte" => Byte
      | _ => raise(InvalidLibraryMode());
    let toString =
      fun
      | Best => "best"
      | Native => "native"
      | Byte => "byte";
  };
  type t = {
    namespace: string,
    modes: option(list(Mode.t)),
    cNames: option(list(string)),
    virtualModules: option(list(string)),
    implements: option(list(string)),
    wrapped: option(bool),
  };
  let create = (namespace, modes, cNames, virtualModules, implements, wrapped) => {
    namespace,
    modes,
    cNames,
    virtualModules,
    implements,
    wrapped,
  };
  let toDuneStanza = (common, lib) => {
    let {name: pkgName, require, path} = common;
    let {
      namespace,
      modes: modesP,
      cNames: cNamesP,
      virtualModules: virtualModulesP,
      implements: implementsP,
      wrapped: wrappedP,
    } = lib;
    let name = Stanza.create("name", Stanza.createAtom(namespace));
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

    let modesD =
      switch (modesP) {
      | None => None
      | Some(l) =>
        Some(
          Stanza.createExpression([
            Stanza.createAtom("modes"),
            ...List.map(m => m |> Mode.toString |> Stanza.createAtom, l),
          ]),
        )
      };

    let cNamesD =
      switch (cNamesP) {
      | None => None
      | Some(l) =>
        Some(
          Stanza.createExpression([
            Stanza.createAtom("c_names"),
            ...List.map(Stanza.createAtom, l),
          ]),
        )
      };

    let virtualModulesD =
      switch (virtualModulesP) {
      | None => None
      | Some(l) =>
        Some(
          Stanza.createExpression([
            Stanza.createAtom("virtual_modules"),
            ...List.map(Stanza.createAtom, l),
          ]),
        )
      };

    let implementsD =
      switch (implementsP) {
      | None => None
      | Some(l) =>
        Some(
          Stanza.createExpression([
            Stanza.createAtom("implements"),
            ...List.map(Stanza.createAtom, l),
          ]),
        )
      };

    let wrappedD =
      switch (wrappedP) {
      | None => None
      | Some(w) =>
        Some(
          Stanza.createExpression([
            Stanza.createAtom("wrapped"),
            Stanza.createAtom(string_of_bool(w)),
          ]),
        )
      };

    let mandatoryExpressions = [name, public_name];
    let optionalExpressions = [
      libraries,
      modesD,
      cNamesD,
      virtualModulesD,
      implementsD,
      wrappedD,
    ];

    let library =
      Stanza.createExpression([
        Stanza.createAtom("library"),
        ...mandatoryExpressions @ filterNone(optionalExpressions),
      ]);

    (path, DuneFile.toString([library]));
  };
};

module Executable: {
  type t;
  module Mode: {
    type t;
    let ofList: list(string) => t;
    let toList: t => list(string);
  };
  let create: (string, option(Mode.t)) => t;
  let toDuneStanza: (common, t) => (string, string);
} = {
  module Mode = {
    exception InvalidCompilationMode(unit);
    exception InvalidBinaryKind(unit);
    module Compilation: {
      type t;
      let toString: t => string;
      let ofString: string => t;
    } = {
      type t =
        | Byte
        | Native
        | Best;

      let toString =
        fun
        | Byte => "byte"
        | Native => "native"
        | Best => "best";

      let ofString =
        fun
        | "byte" => Byte
        | "native" => Native
        | "best" => Best
        | _ => raise(InvalidCompilationMode());
    };

    module BinaryKind: {
      type t;
      let toString: t => string;
      let ofString: string => t;
    } = {
      type t =
        | C
        | Exe
        | Object
        | Shared_object;

      let toString =
        fun
        | C => "c"
        | Exe => "exe"
        | Object => "object"
        | Shared_object => "shared_object";

      let ofString =
        fun
        | "c" => C
        | "exe" => Exe
        | "object" => Object
        | "shared_object" => Shared_object
        | _ => raise(InvalidBinaryKind());
    };

    type t = (Compilation.t, BinaryKind.t);
    exception InvalidExecutableMode(string);
    let ofList = parts =>
      switch (parts) {
      | [c, b] => (Compilation.ofString(c), BinaryKind.ofString(b))
      | _ =>
        raise(
          InvalidExecutableMode(
            sprintf(
              "Invalid executable mode: expected of the form (<compilation mode>, <binary_kind>). Got %s",
              List.fold_left((a, e) => sprintf("%s %s", a, e), "", parts),
            ),
          ),
        )
      };
    let toList = m => {
      let (c, b) = m;
      [Compilation.toString(c), BinaryKind.toString(b)];
    };
  };
  type t = {
    main: string,
    modes: option(Mode.t),
  };
  let create = (main, modes) => {main, modes};
  let toDuneStanza = (common, e) => {
    let {name: pkgName, require, path} = common;
    let {main, modes: modesP} = e;
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

    let modesD =
      switch (modesP) {
      | None => None
      | Some(m) =>
        Some(
          Stanza.createExpression([
            Stanza.createAtom("modes"),
            ...m |> Mode.toList |> List.map(Stanza.createAtom),
          ]),
        )
      };

    let mandatoryExpressions = [name, public_name];
    let optionalExpressions = [libraries, modesD];

    let executable =
      Stanza.createExpression([
        Stanza.createAtom("executable"),
        ...mandatoryExpressions @ filterNone(optionalExpressions),
      ]);

    (path, DuneFile.toString([executable]));
  };
};

type pkgType =
  | ExecutablePackage(Executable.t)
  | LibraryPackage(Library.t);

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

/* */
/*  Inline ppx is supported too. */
/* let%test "getSuffix(): must return suffix" = getSuffix("foo.lib") == "lib"; */
/* */

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
    | Bool(bool)
    | String(string)
    | List(list(t));
  exception ConversionException(string);
  let toBool =
    fun
    | Bool(b) => b
    | String(s) =>
      raise(
        ConversionException(
          sprintf("Expected string. Actual string (%s)", s),
        ),
      )
    | List(l) => raise(ConversionException("Expected string. Actual list"));
  let toString =
    fun
    | String(s) => s
    | Bool(b) =>
      raise(
        ConversionException(
          sprintf("Expected string. Actual bool (%s)", string_of_bool(b)),
        ),
      )
    | List(_) => raise(ConversionException("Expected string. Actual list"));
  let toList =
    fun
    | List(l) => l
    | Bool(b) =>
      raise(
        ConversionException(
          sprintf("Expected list. Actual bool (%s)", string_of_bool(b)),
        ),
      )
    | String(_) =>
      raise(ConversionException("Expected list. Actual string"));
};

/* TODO: Making parsing more lenient? Eg. allow string where single element list is valid */
module JSON: {
  type t;
  let ofString: string => t;
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
  let ofString = jstr => from_string(jstr);
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
    | `Bool(b) => FieldTypes.Bool(b)
    | `String(s) => FieldTypes.String(s)
    | `List(jl) => FieldTypes.List(List.map(j => toValue(j), jl))
    | `Null => raise(NullJSONValue())
    | _ =>
      raise(
        InvalidJSONValue(
          sprintf(
            "Value must be either string, bool or list of string. Found %s",
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
        let modes =
          try (
            Some(
              Executable.Mode.ofList(
                JSON.member(conf, "modes")
                |> JSON.toValue
                |> FieldTypes.toList
                |> List.map(a => a |> FieldTypes.toString),
              ),
            )
          ) {
          | NullJSONValue () => None
          | e => raise(e)
          };
        {
          common: {
            name,
            path: Path.(projectPath / dir),
            require,
          },
          pkgType: ExecutablePackage(Executable.create(main, modes)),
        };
      | _ =>
        let namespace =
          JSON.member(conf, "namespace")
          |> JSON.toValue
          |> FieldTypes.toString;
        let libraryModes =
          try (
            Some(
              JSON.member(conf, "modes")
              |> JSON.toValue
              |> FieldTypes.toList
              |> List.map(FieldTypes.toString)
              |> List.map(Library.Mode.ofString),
            )
          ) {
          | NullJSONValue () => None
          | e => raise(e)
          };
        let cStubs =
          try (
            Some(
              JSON.member(conf, "cNames")
              |> JSON.toValue
              |> FieldTypes.toList
              |> List.map(FieldTypes.toString),
            )
          ) {
          | NullJSONValue () => None
          | e => raise(e)
          };
        let virtualModules =
          try (
            Some(
              JSON.member(conf, "virtualModules")
              |> JSON.toValue
              |> FieldTypes.toList
              |> List.map(FieldTypes.toString),
            )
          ) {
          | NullJSONValue () => None
          | e => raise(e)
          };
        let implements =
          try (
            Some(
              JSON.member(conf, "implements")
              |> JSON.toValue
              |> FieldTypes.toList
              |> List.map(FieldTypes.toString),
            )
          ) {
          | NullJSONValue () => None
          | e => raise(e)
          };
        let wrapped =
          try (
            Some(
              JSON.member(conf, "wrapped")
              |> JSON.toValue
              |> FieldTypes.toBool,
            )
          ) {
          | NullJSONValue () => None
          | e => raise(e)
          };
        {
          common: {
            name,
            path: Path.(projectPath / dir),
            require,
          },
          pkgType:
            LibraryPackage(
              Library.create(
                namespace,
                libraryModes,
                cStubs,
                virtualModules,
                implements,
                wrapped,
              ),
            ),
        };
      };
    },
    pkgs,
  );
};

let toPackages = (_prjPath, pkgs) =>
  List.map(
    pkg =>
      switch (pkg.pkgType) {
      | LibraryPackage(l) => Library.toDuneStanza(pkg.common, l)
      | ExecutablePackage(e) => Executable.toDuneStanza(pkg.common, e)
      },
    pkgs,
  );

let gen = (prjPath, pkgPath) => {
  let json = JSON.fromFile(pkgPath);
  let pesyPackages = toPesyConf(prjPath, json);
  let dunePackages = toPackages(prjPath, pesyPackages); /* TODO: Could return added, updated, deleted files i.e. packages updated so that the main exe could log nicely */
  List.iter(
    dpkg => {
      let (path, duneFile) = dpkg;
      printf("Wrote (%s) \n %s\n", Path.(path / "dune"), duneFile);
      mkdirp(path);
      write(Path.(path / "dune"), duneFile);
    },
    dunePackages,
  );
};

/* TODO: Figure better test setup */
let testToPackages = jsonStr => {
  let json = JSON.ofString(jsonStr);
  let pesyPackages = toPesyConf("", json);
  let dunePackages = toPackages("", pesyPackages);
  List.map(
    p => {
      let (_, d) = p;
      d;
    },
    dunePackages,
  );
};

let%expect_test _ = {
  let duneFiles =
    testToPackages(
      {|
  {
    "buildDirs": {
      "test": {
        "require": ["foo"],
        "main": "Bar",
        "name": "Bar.exe"
      }
    }
  }
       |},
    );
  List.iter(print_endline, duneFiles);
  %expect
  {|
     (executable (name Bar) (public_name Bar.exe) (libraries foo))
   |};
};

let%expect_test _ = {
  let duneFiles =
    testToPackages(
      {|
  {
    "buildDirs": {
      "testlib": {
        "require": ["foo"],
        "namespace": "Foo",
        "name": "bar.lib",
       "modes": ["best"]
      }
    }
  }
       |},
    );
  List.iter(print_endline, duneFiles);
  %expect
  {|
     (library (name Foo) (public_name bar.lib) (libraries foo) (modes best))
   |};
};

let%expect_test _ = {
  let duneFiles =
    testToPackages(
      {|
  {
    "buildDirs": {
      "testlib": {
        "require": ["foo"],
        "namespace": "Foo",
        "name": "bar.lib",
        "cNames": ["stubs"]
      }
    }
  }
       |},
    );
  List.iter(print_endline, duneFiles);
  %expect
  {|
     (library (name Foo) (public_name bar.lib) (libraries foo) (c_names stubs))
   |};
};

let%expect_test _ = {
  let duneFiles =
    testToPackages(
      {|
  {
    "buildDirs": {
      "testlib": {
        "namespace": "Foo",
        "name": "bar.lib",
        "virtualModules": ["foo"]
      }
    }
  }
       |},
    );
  List.iter(print_endline, duneFiles);
  %expect
  {|
     (library (name Foo) (public_name bar.lib) (virtual_modules foo))
   |};
};

let%expect_test _ = {
  let duneFiles =
    testToPackages(
      {|
  {
    "buildDirs": {
      "testlib": {
        "namespace": "Foo",
        "name": "bar.lib",
        "implements": ["foo"]
      }
    }
  }
       |},
    );
  List.iter(print_endline, duneFiles);
  %expect
  {|
     (library (name Foo) (public_name bar.lib) (implements foo))
   |};
};

let%expect_test _ = {
  let duneFiles =
    testToPackages(
      {|
  {
    "buildDirs": {
      "testlib": {
        "namespace": "Foo",
        "name": "bar.lib",
        "wrapped": false
      }
    }
  }
       |},
    );
  List.iter(print_endline, duneFiles);
  %expect
  {|
     (library (name Foo) (public_name bar.lib) (wrapped false))
   |};
};

let%expect_test _ = {
  let duneFiles =
    testToPackages(
      {|
  {
    "buildDirs": {
      "testlib": {
        "main": "Foo",
        "name": "bar.exe",
        "modes": ["best", "c"]
      }
    }
  }
       |},
    );
  List.iter(print_endline, duneFiles);
  %expect
  {|
     (executable (name Foo) (public_name bar.exe) (modes best c))
   |};
};

/* let printAsciiTree = pesyConf => { */
/*   let%lwt _ = */
/*     LTerm.printls( */
/*       LTerm_text.of_string( */
/*         renderAsciiTree( */
/*           "test", */
/*           spf("name:    %s", testMainModuleName), */
/*           spf("main:    %s", testMainModule), */
/*           spf( */
/*             "require: %s", */
/*             List.fold_left((acc, e) => acc ++ " " ++ e, "", testRequire), */
/*           ), */
/*           false, */
/*         ), */
/*       ), */
/*     ); */

/*   let%lwt _ = */
/*     LTerm.printls( */
/*       LTerm_text.of_string( */
/*         renderAsciiTree( */
/*           "library", */
/*           spf("name:      %s", libName), */
/*           spf("namespace: %s", libNamespace), */
/*           spf( */
/*             "require:   %s", */
/*             List.fold_left((acc, e) => acc ++ " " ++ e, "", libRequire), */
/*           ), */
/*           false, */
/*         ), */
/*       ), */
/*     ); */

/*   let%lwt _ = */
/*     LTerm.printls( */
/*       LTerm_text.of_string( */
/*         renderAsciiTree( */
/*           "executable", */
/*           spf("name:      %s", binName), */
/*           spf("main:      %s", binMain), */
/*           spf( */
/*             "require:   %s", */
/*             List.fold_left((acc, e) => acc ++ " " ++ e, "", binRequire), */
/*           ), */
/*           true, */
/*         ), */
/*       ), */
/*     ); */
/* } */
