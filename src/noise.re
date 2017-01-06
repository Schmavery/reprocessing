/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
open Reprocessing;

open P;

open PUtils;

open PConstants;

let setup env => {
  size env 640 360;
  fill env (color 255 0 0);
  0.01
};

let draw z env => {
  background env (color 230 230 250);
  let res = 50;
  let w = float_of_int (width env) /. float_of_int res;
  let h = float_of_int (height env) /. float_of_int res;
  for i in 0 to (res - 1) {
    for j in 0 to (res - 1) {
      fill env (lerpColor white black (noise (0.03 *. float_of_int i) (0.03 *. float_of_int j) z));
      rectf env (float_of_int i *. w) (float_of_int j *. h) w h
    }
  };
  z +. 0.05
};

ReProcessor.run ::setup ::draw ();
