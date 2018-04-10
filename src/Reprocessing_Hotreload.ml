module NoHotreloading = struct
  (* "Hotreload only supported when compiling to bytecode." *)
  let checkRebuild _ =
    false
end

#if BSB_BACKEND = "bytecode" then
  include Reprocessing_Hotreload_Bytecode
#else
  include NoHotreloading
#end
