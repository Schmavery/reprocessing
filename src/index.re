/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
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

let rec init n f acc =>
  switch n {
  | 0 => List.rev acc
  | n => init (n - 1) f [f n, ...acc]
  };

let init n f => init n f [];

let setup env => {
  size env 640 360;
  fill env (color 255 0 0);
  let lst = init 500 (fun v => make (width env));
  /* let fnt = loadFont env "assets/font/font.fnt"; */
  lst
};

let draw lst env => {
  background env (color 230 230 250);
  fill env (color 255 0 0);
  /* ellipse env 200 200 20 30; */
  /* text env fnt "Hello world!!" 20 20; */
  let lst =
    List.map
      (
        fun drop =>
          switch (drop.y + drop.yspeed) {
          | y when y > height env => make (width env)
          | y => {...drop, y}
          }
      )
      lst;
  List.iter
    (
      fun drop => {
        fill env drop.color;
        ellipse env drop.x drop.y (remap drop.z 0 20 1 3) drop.yspeed
      }
    )
    lst;
  /* text env fnt "Hello222 world!!" 20 200; */
  lst
};

ReProcessor.run ::setup ::draw ();
