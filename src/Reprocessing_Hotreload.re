/* include [%matchenv
  switch BSB_BACKEND {
  | "bytecode" => Reprocessing_Hotreload_Native
  | "native" => Reprocessing_Hotreload_Web
  | "js" => Reprocessing_Hotreload_Web
  }
]; */

let checkRebuild = (filePath) =>
  failwith("Hotreload not supported for ios yet.");