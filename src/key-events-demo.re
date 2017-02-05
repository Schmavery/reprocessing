open Reprocessing;

open P;

open PUtils;

type state = {squarePos: (int, int), cmdIsPressed: bool};

let squareWidth = 300;

let squareHeight = 300;

let setup env => {
  size 600 600 env;
  fill (color 255 0 0) env;
  strokeWeight 8 env;
  stroke (color 155 0 0) env;
  {squarePos: (0, 0), cmdIsPressed: false}
};

let draw state env => {
  background (color 150 255 255) env;
  let (sx, sy) = state.squarePos;
  let (px, py) = pmouse env;
  let (x, y) as squarePos =
    if (
      mousePressed env &&
      state.cmdIsPressed && px > sx && px < sx + squareHeight && py > sy && py < sy + squareWidth
    ) {
      let (mx, my) = mouse env;
      let dx = mx - px;
      let dy = my - py;
      (sx + dx, sy + dy)
    } else {
      state.squarePos
    };
  rect x y squareWidth squareHeight env;
  {...state, squarePos}
};

let keyPressed state env =>
  switch (keyCode env) {
  | LeftOsKey => {...state, cmdIsPressed: true}
  | _ => state
  };

let keyReleased state env =>
  switch (keyCode env) {
  | LeftOsKey => {...state, cmdIsPressed: false}
  | _ => state
  };

ReProcessor.run ::setup ::draw ::keyPressed ::keyReleased ();
