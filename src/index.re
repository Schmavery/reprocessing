open Reprocessing;

open P;

open PUtils;

open PConstants;

/* https://www.youtube.com/watch?v=KkyIDI6rQJI
   Purple rain processing demo */
type dropT = {x: int, y: int, z: int, len: int, yspeed: int, color: Common.colorT};

let make w => {
  let z = random 0 20;
  {
    x: random 0 w,
    y: random (-500) (-50),
    z,
    len: remap z 0 20 10 20,
    yspeed: remap z 0 20 5 15,
    color: lerpColor white (color 138 43 226) (randomf 0.3 1.)
  }
};

type state = {lst: list dropT, fnt: Font.Font.t};

let setup env => {
  size env 640 360;
  fill env (color 255 0 0);
  let lst = ref [];
  for i in 0 to 500 {
    lst := [make 640, ...!lst]
  };
  let fnt = loadFont env "assets/font/font.fnt";
  {lst: !lst, fnt}
};

let draw {lst, fnt} env => {
  background env (color 230 230 250);
  text env fnt "Hello world!!" 20 20;
  let lst =
    List.map
      (
        fun drop => {
          let newY = drop.y + drop.yspeed;
          if (newY > height env) {
            /* Make new drop */
            make (width env)
          } else {
            {...drop, y: newY}
          }
        }
      )
      lst;
  List.iter
    (
      fun drop => {
        fill env drop.color;
        ellipse env drop.x drop.y (remap drop.z 0 20 1 5) drop.yspeed
      }
    )
    lst;
  {lst, fnt}
};

ReProcessor.run ::setup ::draw ();
