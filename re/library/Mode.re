module EsyEnv = {
  type t =
    | SHELL
    | BUILD;

  type t' =
    | ESY_ENV(t, string)
    | NAKED;
};
