let getEnv = var =>
  try (Some(Sys.getenv(var))) {
  | Not_found => None
  };
