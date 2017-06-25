open Reprocessing;

/* https://www.youtube.com/watch?v=KkyIDI6rQJI
   Purple rain processing demo */
type dropT = {x: int, y: int, z: int, len: int, yspeed: int, color: Common.colorT, time: int};

let make w (ymin, ymax) time => {
  let z = Utils.random 0 20;
  {
    x: Utils.random 0 w,
    y: Utils.random ymin ymax,
    z,
    len: Utils.remap z 0 20 10 20,
    yspeed: Utils.remap z 0 20 5 15,
    color: Utils.lerpColor Constants.white (Utils.color r::138 g::43 b::226) (Utils.randomf 0.3 1.),
    time
  }
};

type state = {lst: array dropT, running: bool, time: int};

let setup env => {
  Env.size width::640 height::360 env;
  Draw.fill (Utils.color r::255 g::0 b::0) env;
  Draw.noStroke env;
  let lst = Array.init 500 (fun _ => make (Env.width env) ((-500), (-50)) 0);
  {lst, time: 0, running: true}
};

let draw {lst, running, time} env => {
  Draw.background (Utils.color r::230 g::230 b::250) env;
  Draw.fill (Utils.color r::255 g::0 b::0) env;
  Utils.randomSeed time;
  let lst =
    Array.map
      (
        fun drop =>
          switch (drop.y + drop.yspeed * (time - drop.time)) {
          | y when y > Env.height env + 500 => make (Env.width env) ((-500), (-50)) time
          | y when y < (-500) => make (Env.width env) (Env.height env + 50, Env.height env + 500) time
          | _ => drop
          }
      )
      lst;
  Array.iter
    (
      fun drop => {
        Draw.fill drop.color env;
        Draw.ellipse
          center::(drop.x, drop.y + drop.yspeed * (time - drop.time))
          radx::(Utils.remap drop.z 0 20 1 3)
          rady::drop.yspeed
          env
      }
    )
    lst;
  {lst, running, time: running ? time + 1 : time}
};

let mouseDown state _env => {...state, running: false};

let mouseUp state _env => {...state, running: true};

let mouseDragged ({time} as state) env => {
  let (pmouseX, _) = Env.pmouse env;
  let (mouseX, _) = Env.mouse env;
  let newTime = time - (pmouseX - mouseX);
  {...state, time: newTime}
};

run ::setup ::draw ::mouseDown ::mouseUp ::mouseDragged ();
