module NoHotreloading = struct
  let checkRebuild _ =
    failwith "Hotreload only supported when compiling to bytecode."
end

#if BSB_BACKEND = "bytecode" then
  include Reprocessing_Hotreload_Bytecode
#else
  include NoHotreloading
#end
