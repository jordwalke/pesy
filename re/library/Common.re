type t = {
  path: string,
  name: string,
  require: list(string),
  flags: option(list(string)) /* TODO: Use a variant instead since flags only accept a set of values and not any list of strings */
};
let create = (name, path, require, flags) => {name, path, require, flags};
let toDuneStanzas = c => {
  let {name, require, flags, _} = c;
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
  );
};
let getPath = c => c.path;
