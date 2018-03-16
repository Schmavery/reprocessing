module NoHotreloading = struct
  let checkRebuild _ _ =
    false
end

#if BSB_BACKEND = "bytecode" then
  include Reprocessing_Hotreload_Bytecode
#else
  include NoHotreloading
#end
