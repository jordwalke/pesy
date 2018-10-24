exception ProcessInterrupted of string

let contains s1 s2 =
  let re = Str.regexp_string s2 in
  try
    Str.search_forward re s1 0
  with Not_found -> -1

let run_esy_bash ?env_file args_str =
  let existing_vars = Unix.environment () in
  let shell_path = if Sys.unix then "../../bin/EsyBash.exe" else "..\\..\\bin\\EsyBash.exe" in
  let escaping_quotes = if Sys.unix then "'" else "\"" in
  let shell_args =
    let escaped_commands = escaping_quotes ^ args_str ^ escaping_quotes in
    match env_file with
    | Some x -> "--env " ^ x ^ " " ^ escaped_commands
    | None -> escaped_commands in
  let run_process = Unix.open_process_full (shell_path ^ " " ^ shell_args) existing_vars       
  in
  let (p_out_chan, p_in_chan, p_err_chan) = run_process in
  let out_buf = ref "" in
  let err_buf = ref "" in
  let () = try
    while true do
      out_buf := !out_buf ^ (input_line p_out_chan);
    done
  with End_of_file -> () in
  let () = try
      while true do
        err_buf := !err_buf ^ (input_line p_err_chan);
      done
    with End_of_file -> () in
  match (Unix.close_process_full (p_out_chan, p_in_chan, p_err_chan)) with
  | WEXITED c -> (!out_buf, !err_buf, c)
  | WSIGNALED c -> raise (ProcessInterrupted ("Process signalled with " ^ string_of_int c))
  | WSTOPPED c -> raise (ProcessInterrupted ("Process stopped with " ^ string_of_int c));


