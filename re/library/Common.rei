type t;
let toDuneStanzas: t => (Stanza.t, option(Stanza.t), option(Stanza.t));
let getPath: t => string;
let create: (string, string, list(string), option(list(string))) => t;
