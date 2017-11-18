let load_plug = (fname) => {
  let fname = Dynlink.adapt_filename(fname);
  if (Sys.file_exists(fname)) {
    try (Dynlink.loadfile(fname)) {
    | Dynlink.Error(Dynlink.Inconsistent_import(name)) =>
      print_endline("Inconsistent import of " ++ name ++ ". You probably have an error (check bsb).")
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

let ocaml = Dynlink.is_native ? "ocamlopt.opt" : "ocamlc.opt";
/**
 * Get the output of a command, in lines.
 */
let readCommand = (command) => {
  let chan = Unix.open_process_in(command);
  try {
    let rec loop = () => {
      switch (Pervasives.input_line(chan)) {
      | exception End_of_file => []
      | line => [line, ...loop()]
      }
    };
    let lines = loop();
    switch (Unix.close_process_in(chan)) {
    | WEXITED(0) => Some(lines)
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

let getSourceNames = (mainFile) => {
  let sourceDirectory = Filename.dirname(mainFile);
  List.filter(
    (name) => Filename.check_suffix(name, ".re"),
    readdir(sourceDirectory)
  )
  |> List.map((name) => sourceDirectory ++ "/" ++ name)
};

let folder = Dynlink.is_native ? "native" : "bytecode";

let extension = Dynlink.is_native ? "cmxs" : "cmo";

let withReSuffix = path => Filename.chop_extension(path) ++ ".re";

let parseOcamldep = (lines) => {
  List.map(
    line => {
      switch (Str.split(Str.regexp(":"), line)) {
      | [target, deps] => {
        let target = withReSuffix(String.trim(target));
        let deps =
        Str.split(Str.regexp(" "), deps)
        |> List.map(String.trim)
        |> List.filter(x => String.length(x) > 0)
        |> List.map(withReSuffix)
        ;
        (target, deps)
      }
      | [target] => {
        (target |> String.trim |> withReSuffix, [])
      }
      | _ => failwith("Invalid ocamldep output: " ++ line)
      }
    },
    lines
  );
};

let oneFilesDeps = (depsList, mainFile) => {
  let touched = Hashtbl.create(List.length(depsList));
  let rec loop = (fname) => {
    Hashtbl.add(touched, fname, true);
    let deps = try (List.assoc(fname, depsList)) {
    | Not_found => failwith("Dependency not listed by ocamldep: " ++ fname)
    }
    ;
    List.iter(loop, deps);
  };
  loop(mainFile);
  List.filter(item => Hashtbl.mem(touched, fst(item)), depsList);
};

let resolveDependencyOrder = (depsList, mainFile) => {
  let mainDeps = oneFilesDeps(depsList, mainFile);
  let scores = Hashtbl.create(List.length(mainDeps));

  /** Initialize everything to zero */
  List.iter(
    ((target, deps)) => {
        Hashtbl.add(scores, target, 0);
        List.iter(name => Hashtbl.add(scores, name, 0), deps);
    },
    mainDeps
  );

  let loop = () => {
    List.fold_left(
      (updated, (target, deps)) => {
        let highestDep = List.fold_left(
          (highest, dep) => max(highest, Hashtbl.find(scores, dep)),
          0,
          deps
        );
        let current = Hashtbl.find(scores, target);
        if (current < highestDep + 1) {
          Hashtbl.add(scores, target, highestDep + 1);
          true
        } else {
          updated
        }
      },
      false,
      mainDeps
    )
  };

  /* this should settle pretty quickly */
  while (loop()) ();

  let files = List.map(fst, mainDeps);
  let sorted = List.sort((a, b) => Hashtbl.find(scores, a) - Hashtbl.find(scores, b), files);
  sorted
};

let ocamlBase = "node_modules/bs-platform/vendor/ocaml/";

let sortSourceFilesInDependencyOrder = (sourceFiles, mainFile) => {
  let cmd =
    Printf.sprintf(
      "%s -pp './node_modules/bs-platform/bin/refmt3.exe --print binary' -ml-synonym .re -I %s -one-line -native %s",
      ocamlBase ++ "bin/ocamlrun " ++ ocamlBase ++ "bin/ocamldep",
      Filename.dirname(mainFile),
      String.concat(" ", sourceFiles)
    );
  switch (readCommand(cmd)) {
  | None => None
  | Some(raw) =>
    let depsList = parseOcamldep(raw);
    let filesInOrder = resolveDependencyOrder(depsList, mainFile);
    Some(filesInOrder)
  }
};

let lastModifiedTimes = Hashtbl.create(10);

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
        if (Hashtbl.mem(lastModifiedTimes, name)) {
          if (st_mtime > Hashtbl.find(lastModifiedTimes, name)) {
            Hashtbl.add(lastModifiedTimes, name, st_mtime);
            (true, notReady)
          } else {
            (needsRebuild, notReady)
          }
        } else {
          Hashtbl.add(lastModifiedTimes, name, st_mtime);
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

/**
 * Every frame, we check to see if any source files have updated `.cmo`s
 *
 * If there are, then we use ocamldep to determine dependency order, and load them
 * in that order.
 */
let checkRebuild = (mainFile) => {
  let sourceNames = getSourceNames(mainFile);
  let (needsRebuild, notReady) = needsRebuild(List.map(compiledPath, sourceNames));
  if (! needsRebuild || notReady) {
    false
  } else {
    switch (sortSourceFilesInDependencyOrder(sourceNames, mainFile)) {
    | None => false
    | Some(filesInOrder) =>
      List.iter((name) => load_plug(compiledPath(name)), filesInOrder);
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
        "%s %s -w -40 -I lib/bs/%s/src -I node_modules/ReasonglInterface/lib/bs/%s/src -I node_modules/Reprocessing/lib/bs/%s/src -pp './node_modules/bs-platform/lib/refmt3.exe --print binary' -o lib/bs/%s/%s.%s -impl %s 2>&1",
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
