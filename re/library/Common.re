type t = {
  path: string,
  name: string,
  require: list(string),
  flags: option(list(string)), /* TODO: Use a variant instead since flags only accept a set of values and not any list of strings */
  ocamlcFlags: option(list(string)),
  ocamloptFlags: option(list(string)),
  jsooFlags: option(list(string)),
};
let create =
    (name, path, require, flags, ocamlcFlags, ocamloptFlags, jsooFlags) => {
  name,
  path,
  require,
  flags,
  ocamlcFlags,
  ocamloptFlags,
  jsooFlags,
};
let toDuneStanzas = c => {
  let {name, require, flags, ocamlcFlags, ocamloptFlags, jsooFlags, _} = c;
  (
    /* public_name: */ Stanza.create("public_name", Stanza.createAtom(name)),
    /* libraries: */
    switch (require) {
    | [] => None
    | libs =>
      Some(
        Stanza.createExpression([
          Stanza.createAtom("libraries"),
          ...List.map(r => Stanza.createAtom(r), libs),
        ]),
      )
    },
    /* flags: */
    switch (flags) {
    | None => None
    | Some(l) =>
      Some(
        Stanza.createExpression([
          Stanza.createAtom("flags"),
          ...List.map(f => Stanza.createAtom(f), l),
        ]),
      )
    },
    /* ocamlcFlags */
    switch (ocamlcFlags) {
    | None => None
    | Some(l) =>
      Some(
        Stanza.createExpression([
          Stanza.createAtom("ocamlc_flags"),
          ...List.map(f => Stanza.createAtom(f), l),
        ]),
      )
    },
    /* ocamloptFlags */
    switch (ocamloptFlags) {
    | None => None
    | Some(l) =>
      Some(
        Stanza.createExpression([
          Stanza.createAtom("ocamlopt_flags"),
          ...List.map(f => Stanza.createAtom(f), l),
        ]),
      )
    },
    /* jsooFlags */
    switch (jsooFlags) {
    | None => None
    | Some(l) =>
      Some(
        Stanza.createExpression([
          Stanza.createAtom("js_of_ocaml"),
          ...List.map(f => Stanza.createAtom(f), l),
        ]),
      )
    },
  );
};
let getPath = c => c.path;
