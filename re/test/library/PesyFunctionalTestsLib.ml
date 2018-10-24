open PesyUtils
  
let%test "kebab: Must turn a string like EsyBash to esy-bash" =
  (kebab "EsyBash") = "esy-bash"
    
let%test "getCurrentDirName: Must return current directory name" =
  Unix.putenv "PWD" "/home/foo-bar";
  (getCurrentDirName ()) = "foo-bar"


let%test "removeScope: Must strip package scop from the name" =
  (removeScope ("@opam/foo-bar")) = "foo-bar" &&
  (removeScope ("@opam/fooBar")) = "fooBar"
