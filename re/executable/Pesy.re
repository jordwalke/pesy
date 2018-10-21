let isInsideEsyEnv = () =>
  switch (PesyUtils.getEnv("cur__name")) {
  | Some(_) => true
  | None => false
  };

if (isInsideEsyEnv()) {
  /* then run build */
  Lwt_main.run(PesyLib.build());
} else {
  Lwt_main.run(PesyLib.bootstrap());
};
