open PesyUtils
  
let%test "kebab: Must turn a string like EsyBash to esy-bash" =
  (kebab "EsyBash") = "esy-bash"
    
let%test "getCurrentDirName: Must return current directory name" =
  Unix.putenv "PWD" "/home/foo-bar";
  (getCurrentDirName ()) = "foo-bar"


let%test "removeScope: Must strip package scop from the name" =
  (removeScope ("@opam/foo-bar")) = "foo-bar" &&
  (removeScope ("@opam/fooBar")) = "fooBar"


let%test "readFile: Return file contents" =
  let test_file = "test.dat" in
  let message = "testing123" in
  let oc = open_out test_file in
  Printf.fprintf oc "%s\n" message;
  close_out oc;
  (readFile test_file) = "testing123"
  
