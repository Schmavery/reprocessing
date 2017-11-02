open Reprocessing;

/* https://www.youtube.com/watch?v=KkyIDI6rQJI
   Purple rain processing demo */
type dropT = {
  x: int,
  y: int,
  z: int,
  len: int,
  yspeed: int,
  color: colorT,
  time: int
};

let make = (w, (ymin, ymax), time) => {
  let z = Utils.random(~min=0, ~max=20);
  {
    x: Utils.random(~min=0, ~max=w),
    y: Utils.random(~min=ymin, ~max=ymax),
    z,
    len: Utils.remap(~value=z, ~low1=0, ~high1=20, ~low2=10, ~high2=20),
    yspeed: Utils.remap(~value=z, ~low1=0, ~high1=20, ~low2=5, ~high2=15),
    color:
      Utils.lerpColor(
        ~low=Constants.white,
        ~high=Utils.color(~r=138, ~g=43, ~b=226, ~a=255),
        ~value=Utils.randomf(~min=0.3, ~max=1.)
      ),
    time
  }
};

type state = {
  lst: array(dropT),
  running: bool,
  time: int
};

let setup = (env) => {
  Env.size(~width=640, ~height=360, env);
  Draw.fill(Utils.color(~r=255, ~g=0, ~b=0, ~a=255), env);
  Draw.noStroke(env);
  let lst = Array.init(500, (_) => make(Env.width(env), ((-500), (-50)), 0));
  {lst, time: 0, running: true}
};

let draw = ({lst, running, time}, env) => {
  Draw.background(Utils.color(~r=230, ~g=230, ~b=250, ~a=255), env);
  Draw.fill(Utils.color(~r=255, ~g=0, ~b=0, ~a=255), env);
  Utils.randomSeed(time);
  let lst =
    Array.map(
      (drop) =>
        switch (drop.y + drop.yspeed * (time - drop.time)) {
        | y when y > Env.height(env) + 500 => make(Env.width(env), ((-500), (-50)), time)
        | y when y < (-500) =>
          make(Env.width(env), (Env.height(env) + 50, Env.height(env) + 500), time)
        | _ => drop
        },
      lst
    );
  Array.iter(
    (drop) => {
      Draw.fill(drop.color, env);
      Draw.ellipse(
        ~center=(drop.x, drop.y + drop.yspeed * (time - drop.time)),
        ~radx=Utils.remap(~value=drop.z, ~low1=0, ~high1=20, ~low2=1, ~high2=3),
        ~rady=drop.yspeed,
        env
      )
    },
    lst
  );
  {lst, running, time: running ? time + 1 : time}
};

let mouseDown = (state, _env) => {...state, running: false};

let mouseUp = (state, _env) => {...state, running: true};

let mouseDragged = ({time} as state, env) => {
  let (pmouseX, _) = Env.pmouse(env);
  let (mouseX, _) = Env.mouse(env);
  let newTime = time - (pmouseX - mouseX);
  {...state, time: newTime}
};

run(~setup, ~draw, ~mouseDown, ~mouseUp, ~mouseDragged, ());
