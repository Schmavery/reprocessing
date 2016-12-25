open Reprocessing;
open P;
open PUtils;

let setup env => {
  size env 600 600;
  fill env (color 255 0 0);
};

let draw user env => {
  rect env 100 100 300 300;
  user
};

ReProcessor.run ::setup ::draw ();
