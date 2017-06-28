module type TypesT = {
  type glEnvT = Reprocessing_Common.glEnv;
  type colorT = Reprocessing_Common.colorT;
  type imageT = Reprocessing_Common.imageT;
  type fontT = Reprocessing_Font.fontT;
  type strokeCapT = Reprocessing_Common.strokeCapT;
};

module Types: TypesT = {
  type glEnvT = Reprocessing_Common.glEnv;
  type colorT = Reprocessing_Common.colorT;
  type imageT = Reprocessing_Common.imageT;
  type fontT = Reprocessing_Font.fontT;
  type strokeCapT = Reprocessing_Common.strokeCapT;
};
