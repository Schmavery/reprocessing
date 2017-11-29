let load_plug = (fname) => {
  let fname = Dynlink.adapt_filename(fname);
  if (Sys.file_exists(fname)) {
    try (Dynlink.loadfile(fname)) {
    | Dynlink.Error(err) as e =>
      print_endline("ERROR loading plugin: " ++ Dynlink.error_message(err));
      raise(e)
    | e =>
      print_endline("Unknown error while loading plugin");
      raise(e)
    }
  } else {
    failwith("Plugin file does not exist")
  }
};

let readdir = (dir) => {
  let maybeGet = (handle) =>
    try (Some(Unix.readdir(handle))) {
    | End_of_file => None
    };
  let rec loop = (handle) =>
    switch (maybeGet(handle)) {
    | None =>
      Unix.closedir(handle);
      []
    | Some(name) => [name, ...loop(handle)]
    };
  loop(Unix.opendir(dir))
};

let readCommand = (command) => {
  print_endline(command);
  let chan = Unix.open_process_in(command);
  try {
    let line = Pervasives.input_line(chan);
    print_endline(line);
    switch (Unix.close_process_in(chan)) {
    | WEXITED(0) => Some(line)
    | WEXITED(_)
    | WSIGNALED(_)
    | WSTOPPED(_) =>
      print_endline("Unable to determine dependency order of files");
      None
    }
  } {
  | End_of_file => None
  }
};

let getSourceNames = (hotFile) => {
  let sourceDirectory = Filename.dirname(hotFile);
  let hotName = Filename.basename(hotFile);
  List.filter(
    (name) => name != hotName && Filename.check_suffix(name, ".re"),
    readdir(sourceDirectory)
  )
  |> List.map((name) => sourceDirectory ++ "/" ++ name)
};

let folder = Dynlink.is_native ? "native" : "bytecode";

let extension = Dynlink.is_native ? "cmxs" : "cmo";

let ocamlBase = "node_modules/bs-platform/vendor/ocaml/";

let sortSourceFilesInDependencyOrder = (sourceFiles) => {
  let cmd =
    Printf.sprintf(
      "%s -sort -pp './node_modules/bs-platform/bin/refmt3.exe --print binary' -ml-synonym .re %s",
      ocamlBase ++ "bin/ocamlrun " ++ ocamlBase ++ "bin/ocamldep",
      String.concat(" ", sourceFiles)
    );
  switch (readCommand(cmd)) {
  | None => None
  | Some(raw) =>
    let names = Str.split(Str.regexp(" "), raw);
    Some(names)
  }
};

module StringTbl =
  Hashtbl.Make(
    {
      type t = string;
      let equal = (a, b) => a == b;
      let hash = Hashtbl.hash;
    }
  );

let lastModifiedTimes = StringTbl.create(10);

let needsRebuild = (fileNames) =>
  List.fold_left(
    ((needsRebuild, notReady), name) => {
      /* If a file hasn't been compiled that we expect to be there, we set `notReady` to true
       * As soon as bucklescript has built it, it will be ready. */
      let mtime =
        try (Some(Unix.stat(name).Unix.st_mtime)) {
        | Unix.Unix_error(Unix.ENOENT, _, _) => None
        };
      switch mtime {
      | None => (needsRebuild, true)
      | Some(st_mtime) =>
        if (StringTbl.mem(lastModifiedTimes, name)) {
          if (st_mtime > StringTbl.find(lastModifiedTimes, name)) {
            StringTbl.add(lastModifiedTimes, name, st_mtime);
            (true, notReady)
          } else {
            (needsRebuild, notReady)
          }
        } else {
          StringTbl.add(lastModifiedTimes, name, st_mtime);
          (true, notReady)
        }
      }
    },
    (false, false),
    fileNames
  );

let compiledPath = (name) => {
  let bare = Filename.chop_extension(name);
  Printf.sprintf("lib/bs/%s/%s.%s", folder, bare, extension)
};

let checkRebuild = (hotFile) => {
  let sourceNames = getSourceNames(hotFile);
  let (needsRebuild, notReady) = needsRebuild(List.map(compiledPath, sourceNames));
  if (! needsRebuild || notReady) {
    false
  } else {
    switch (sortSourceFilesInDependencyOrder(sourceNames)) {
    | None => false
    | Some(filesInOrder) =>
      List.iter((name) => load_plug(compiledPath(name)), filesInOrder);
      print_endline("loaded fools");
      true
    }
  }
};

/** TODO remove */
let last_st_mtime = ref(0.);

let ocaml = Dynlink.is_native ? "ocamlopt" : "ocamlc";

let shared = Dynlink.is_native ? "-shared" : "-c";

let ocamlPath = ocamlBase ++ ocaml;

let checkRebuildSingle = (filePath) => {
  let {Unix.st_mtime} = Unix.stat(filePath);
  if (st_mtime > last_st_mtime^) {
    print_endline("Rebuilding hotloaded module");
    /* @Incomplete Check the error code sent back. Also don't do this, use stdout/stderr. */
    let cmd =
      Printf.sprintf(
        "%s %s -w -40 -I lib/bs/%s/src -I node_modules/ReasonglInterface/lib/bs/%s/src -I node_modules/Reprocessing/lib/bs/%s/src -pp './node_modules/bs-platform/bin/refmt3.exe --print binary' -o lib/bs/%s/%s.%s -impl %s 2>&1",
        ocamlPath,
        shared,
        /* folder */
        folder,
        folder,
        folder,
        folder,
        filePath,
        extension,
        filePath
      );
    /* print_endline cmd; */
    switch (Unix.system(cmd)) {
    | WEXITED(0) => ()
    | WEXITED(_)
    | WSIGNALED(_)
    | WSTOPPED(_) => print_endline("Hotreload failed")
    };
    /*Unix.system "./node_modules/.bin/bsb";*/
    load_plug @@ Printf.sprintf("lib/bs/%s/%s.%s", folder, filePath, extension);
    last_st_mtime := st_mtime;
    /* print_endline "----------------------"; */
    true
  } else {
    false
  }
};