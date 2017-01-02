open Reprocessing;

open P;

open PUtils;

open PConstants;

/* https://www.youtube.com/watch?v=KkyIDI6rQJI
   Purple rain processing demo */
type dropT = {x: int, y: int, z: int, len: int, yspeed: int, color: color};

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

let setup env => {
  size env 640 360;
  fill env (color 255 0 0);
  let lst = ref [];
  for i in 0 to 500 {
    lst := [make 640, ...!lst]
  };
  !lst
};

let draw lst env => {
  background env (color 230 230 250);
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
        strokeWeight env (remap drop.z 0 20 1 5);
        stroke env drop.color;
        line env (drop.x, drop.y) (drop.x, drop.y + drop.len)
      }
    )
    lst;
  ellipse env 200 200 100 50;
  lst
};

ReProcessor.run ::setup ::draw ();
