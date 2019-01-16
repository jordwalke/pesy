type t;
let toDuneStanzas:
  t =>
  (
    Stanza.t,
    option(Stanza.t),
    option(Stanza.t),
    option(Stanza.t),
    option(Stanza.t),
    option(Stanza.t),
  );
let getPath: t => string;
let create:
  (
    /* name */ string,
    /* path */ string,
    /* require */ list(string),
    /* flags */ option(list(string)),
    /* Ocamlc flags */ option(list(string)),
    /* Ocamlopt flags */ option(list(string)),
    /* jsoo flags */ option(list(string))
  ) =>
  t;
