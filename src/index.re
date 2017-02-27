open Reprocessing;

open P;

open PUtils;

open PConstants;

/* https://www.youtube.com/watch?v=KkyIDI6rQJI
   Purple rain processing demo */
type dropT = {x: int, y: int, z: int, len: int, yspeed: int, color: Common.colorT, time: int};

let make w (ymin, ymax) time => {
  let z = random 0 20;
  {
    x: random 0 w,
    y: random ymin ymax,
    z,
    len: remap z 0 20 10 20,
    yspeed: remap z 0 20 5 15,
    color: lerpColor white (color 138 43 226) (randomf 0.3 1.),
    time
  }
};

type state = {lst: list dropT, running: bool, time: int};

let rec init n f acc =>
  switch n {
  | 0 => List.rev acc
  | n => init (n - 1) f [f n, ...acc]
  };

let init n f => init n f [];

let setup env => {
  size 640 360 env;
  fill (color 255 0 0) env;
  noStroke env;
  let lst = init 500 (fun v => make (width env) ((-500), (-50)) 0);
  {lst, time: 0, running: true}
};

let draw {lst, running, time} env => {
  background (color 230 230 250) env;
  /* fill (color 255 0 0) env;
  randomSeed time;
  let lst =
    List.map
      (
        fun drop =>
          switch (drop.y + drop.yspeed * (time - drop.time)) {
          | y when y > height env + 500 => make (width env) ((-500), (-50)) time
          | y when y < (-500) => make (width env) (height env + 50, height env + 500) time
          | _ => drop
          }
      )
      lst;
  List.iter
    (
      fun drop => {
        fill drop.color env;
        ellipse
          (drop.x, drop.y + drop.yspeed * (time - drop.time))
          (remap drop.z 0 20 1 3)
          drop.yspeed
          env
      }
    )
    lst; */
  stroke (color 0 0 0) env;
  strokeWeight 1   env;
  bezier (0., 0.) (100., 100.) (200., 100.) (200., 200.0) env;
  {lst, running, time: running ? time + 1 : time}
};

let mouseDown ({running} as state) env => {...state, running: false};

let mouseUp ({running} as state) env => {...state, running: true};

let mouseDragged ({time} as state) env => {
  let (pmouseX, _) = pmouse env;
  let (mouseX, _) = mouse env;
  let newTime = time - (pmouseX - mouseX);
  {...state, time: newTime}
};

ReProcessor.run ::setup ::draw ::mouseDown ::mouseUp ::mouseDragged ();
