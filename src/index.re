open Reprocessing;

open P;

open PUtils;

/* https://www.youtube.com/watch?v=KkyIDI6rQJI
  Purple rain processing demo */

type dropT = {x: int, y: int, z: int, len: int, yspeed: int};

let setup env => {
  size env 600 600;
  fill env (color 255 0 0);
  let lst = ref [];
  let make () => {
    x: Random.int 600,
    y: - (Random.int 450 + 50),
    z: Random.int 20,
    len: Random.int 20,
    yspeed: Random.int 20
  };
  for i in 0 to 500 {
    lst := [make (), ...!lst]
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
          if (newY > 600) {
            {...drop, y: - (Random.int 450 + 50), yspeed: Random.int 20}
          } else {
            {...drop, y: newY}
          }
        }
      )
      lst;
  List.iter
    (
      fun drop => {
        lineWeight env 1;
        stroke env (color 138 43 226);
        line env (drop.x, drop.y) (drop.x, drop.y + drop.len)
      }
    )
    lst;
  lst
};

ReProcessor.run ::setup ::draw ();
