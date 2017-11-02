let load_plug fname => {
  let fname = Dynlink.adapt_filename fname;
  if (Sys.file_exists fname) {
    try (Dynlink.loadfile fname) {
    | Dynlink.Error err as e =>
      print_endline ("ERROR loading plugin: " ^ Dynlink.error_message err);
      raise e
    | _ => failwith "Unknown error while loading plugin"
    }
  } else {
    failwith "Plugin file does not exist"
  }
};

let last_st_mtime = ref 0.;

let ocaml = Dynlink.is_native ? "ocamlopt" : "ocamlc";

let extension = Dynlink.is_native ? "cmxs" : "cmo";

let shared = Dynlink.is_native ? "-shared" : "-c";

let folder = Dynlink.is_native ? "native" : "bytecode";

let ocamlPath = "node_modules/bs-platform/vendor/ocaml/" ^ ocaml;

let checkRebuild filePath => {
  let {Unix.st_mtime: st_mtime} = Unix.stat filePath;
  if (st_mtime > !last_st_mtime) {
    print_endline "Rebuilding hotloaded module";
    /* @Incomplete Check the error code sent back. Also don't do this, use stdout/stderr. */
    let cmd =
      Printf.sprintf
        "%s %s -w -40 -I lib/bs/%s/src -I node_modules/ReasonglInterface/lib/bs/%s/src -I node_modules/Reprocessing/lib/bs/%s/src -pp './node_modules/bs-platform/bin/refmt.exe --print binary' -o lib/bs/%s/%s.%s -impl %s 2>&1"
        ocamlPath
        shared
        /* folder */
        folder
        folder
        folder
        folder
        filePath
        extension
        filePath;
    /* print_endline cmd; */
    switch (Unix.system cmd) {
    | WEXITED 0 => ()
    | WEXITED _
    | WSIGNALED _
    | WSTOPPED _ => print_endline "Hotreload failed"
    };
    /*Unix.system "./node_modules/.bin/bsb";*/
    load_plug @@ Printf.sprintf "lib/bs/%s/%s.%s" folder filePath extension;
    last_st_mtime := st_mtime;
    /* print_endline "----------------------"; */
    true
  } else {
    false
  }
};
