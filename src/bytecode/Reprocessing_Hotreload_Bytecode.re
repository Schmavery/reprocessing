let load_plug = fname => {
  let fname = Dynlink.adapt_filename(fname);
  if (Sys.file_exists(fname)) {
    try (Dynlink.loadfile(fname)) {
    | Dynlink.Error(err) =>
      print_endline("ERROR loading plugin: " ++ Dynlink.error_message(err))
    | e => failwith("Unknown error while loading plugin: " ++ Printexc.to_string(e))
    };
  } else {
    failwith("Plugin file does not exist");
  };
};

let last_st_mtime = ref(0.);

let extension = Sys.win32 || Sys.cygwin ? ".exe" : "";

let ocaml = (Dynlink.is_native ? "ocamlopt.opt" : "ocamlc.opt") ++ extension;

let extension = Dynlink.is_native ? "cmxs" : "cmo";

let shared = Dynlink.is_native ? "-shared" : "-c";

let folder = Dynlink.is_native ? "native" : "bytecode";

let (+/) = Filename.concat;

let ocamlPath =
  "node_modules" +/ "bs-platform" +/ "vendor" +/ "ocaml" +/ ocaml;

let refmtexe = "node_modules" +/ "bs-platform" +/ "lib" +/ "refmt3.exe";

let bsb = "node_modules" +/ ".bin" +/ "bsb";

let checkRebuild = (firstTime, filePath) => {
  if (firstTime) {
    /* Compile once synchronously because we're going to load it immediately after this. */
    switch (
      Unix.system(
        bsb
        ++ " -build-library "
        ++ String.capitalize(
             Filename.chop_extension(Filename.basename(filePath))
           )
      )
    ) {
    | WEXITED(0) => ()
    | WEXITED(_)
    | WSIGNALED(_)
    | WSTOPPED(_) => print_endline("Hotreload failed")
    };
    let pid =
      Unix.create_process(
        bsb,
        [|
          bsb,
          "-w",
          "-build-library",
          String.capitalize(
            Filename.chop_extension(Filename.basename(filePath))
          )
        |],
        Unix.stdin,
        Unix.stdout,
        Unix.stderr
      );
    print_endline("bsb running with pid: " ++ string_of_int(pid));
    /* 9 is SIGKILL */
    at_exit(() => Unix.kill(pid, 9));
    ();
  };
  let filePath = "lib" +/ "bs" +/ "bytecode" +/ "lib.cma";
  if (Sys.file_exists(filePath)) {
    let {Unix.st_mtime} = Unix.stat(filePath);
    if (st_mtime > last_st_mtime^) {
      print_endline("Reloading hotloaded module");
      load_plug(filePath);
      last_st_mtime := st_mtime;
      true;
    } else {
      false;
    };
  } else {
    false;
  };
};
