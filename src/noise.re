open Reprocessing;

open P;

open PUtils;

open PConstants;

let setup env => {
  size env 600 300;
  fill env (color 255 0 0);
  0.01
};

let draw z env => {
  background env (color 230 230 250);
  let w = width env / 50;
  let h = height env / 50;
  for i in 0 to 49 {
    for j in 0 to 49 {
      fill env (lerpColor white black (noise (0.03 *. float_of_int i) (0.03 *. float_of_int j) z));
      rect env (i * w) (j * h) w h
    }
  };
  z +. 0.1
};

ReProcessor.run ::setup ::draw ();
