let load_plug = fname => {
  let fname = Dynlink.adapt_filename(fname);
  if (Sys.file_exists(fname)) {
    try (Dynlink.loadfile(fname)) {
    | Dynlink.Error(err) as e =>
      print_endline("ERROR loading plugin: " ++ Dynlink.error_message(err));
      raise(e);
    | _ => failwith("Unknown error while loading plugin")
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

let compile = m =>
  while (true) {
    Mutex.lock(m);
    switch (Unix.system("~/Desktop/bucklescript/lib/bsb -build-library Index")) {
    | WEXITED(0) => ()
    | WEXITED(_)
    | WSIGNALED(_)
    | WSTOPPED(_) => print_endline("Hotreload failed")
    };
    Mutex.unlock(m);
  };

let m = Mutex.create();
let unlockMutex = () => Mutex.unlock(m);
let checkRebuild = (firstTime, filePath) => {
  if (firstTime) {
    switch (Unix.system("~/Desktop/bucklescript/lib/bsb -build-library Index")) {
    | WEXITED(0) => ()
    | WEXITED(_)
    | WSIGNALED(_)
    | WSTOPPED(_) => print_endline("Hotreload failed")
    };
    Mutex.lock(m);
    ignore @@ Thread.create(compile, m);
    load_plug("./lib/bs/bytecode/lib.cma");
    true;
  } else {
    let filePath = "./lib/bs/bytecode/lib.cma";
    if (Sys.file_exists(filePath)) {
      let {Unix.st_mtime} = Unix.stat(filePath);
      if (st_mtime > last_st_mtime^) {
        print_endline("Rebuilding hotloaded module");
        Mutex.lock(m);
        load_plug(filePath);
        Mutex.unlock(m);
        last_st_mtime := st_mtime;
        true;
      } else {
        false;
      };
    } else {
      false;
    };
  };
};
