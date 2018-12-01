open LTerm_style;

open LTerm_text;

let esyNotInstalledErrorMsg = "ERROR: You haven't installed esy globally. First install esy then try again";

let esyNotInstalledError =
  eval([B_fg(red), S(esyNotInstalledErrorMsg), E_fg]);

let projectExistsErrorMsgTitle = {|
  Incorrect Usage:
|};

let projectExistsErrorMsgBody = {|
  pesy should only be run via 'esy pesy', unless you want to create a new project
  in a fresh directory. This does not appear to be a fresh directory because
  it contains a package.json or esy.json file
  Type esy --help for more assistance
|};

let projectExistsError =
  eval([
    B_fg(red),
    S(projectExistsErrorMsgTitle),
    E_fg,
    S(projectExistsErrorMsgBody),
  ]);

let readmeExistsWarning =
  eval([
    B_fg(yellow),
    S("README.md already exists. Skipping README generation"),
    E_fg,
  ]);

let gitignoreExistsWarning =
  eval([
    B_fg(yellow),
    S(".gitignore already exists. Skipping .gitignore generation."),
    E_fg,
  ]);

let todo = eval([B_fg(red), S("TODO"), E_fg]);
