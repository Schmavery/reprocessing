open Reprocessing.Utils;

open Reprocessing.Draw;

open Reprocessing.Env;

open Reprocessing.Constants;

let setup = (env) => {
  size(~width=640, ~height=640, env);
  noStroke(env);
  0.01
};

let draw = (z, env) => {
  background(color(~r=230, ~g=230, ~b=250, ~a=255), env);
  let res = 100;
  let w = float_of_int(width(env)) /. float_of_int(res);
  let h = float_of_int(height(env)) /. float_of_int(res);
  for (i in 0 to res - 1) {
    for (j in 0 to res - 1) {
      fill(
        lerpColor(
          ~low=white,
          ~high=black,
          ~value=noise(0.03 *. float_of_int(i), 0.03 *. float_of_int(j), z)
        ),
        env
      );
      rectf(~pos=(float_of_int(i) *. w, float_of_int(j) *. h), ~width=w, ~height=h, env)
    }
  };
  z +. 0.05
};

Reprocessing.run(~setup, ~draw, ());
