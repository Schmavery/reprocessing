include [%matchenv
  switch BSB_BACKEND {
  | "bytecode" => Reprocessing_Hotreload_Native
  | "native" => Reprocessing_Hotreload_Web
  | "js" => Reprocessing_Hotreload_Web
  }
];
