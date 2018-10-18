open LTerm_style;

open LTerm_text;

let build = () => LTerm.printls(LTerm_text.of_string("TODO: Build process"));

let isEsyInstalled = () => true;

let bootstrap = () =>
  if (!isEsyInstalled()) {
    LTerm.printls(eval([B_fg(red), S(Copy.esyNotInstalledError), E_fg]));
  } else {
    LTerm.printls(LTerm_text.of_string("TODO: Bootstrap"));
  };

module Utils = {
  let getEnv = var =>
    try (Some(Sys.getenv(var))) {
    | Not_found => None
    };
};
