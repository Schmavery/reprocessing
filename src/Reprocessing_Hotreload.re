let module NoHotreloading = {
  let checkRebuild = (filePath) =>
  failwith("Hotreload not supported for native and web, compile to bytecode.");
};
include [%matchenv
  switch BSB_BACKEND {
  | "bytecode" => Reprocessing_Hotreload_Native
  | "native" => Reprocessing_Hotreload_Web
  | "js" => Reprocessing_Hotreload_Web
  | "native-android" => NoHotreloading
  | "native-ios" => NoHotreloading
  }
];