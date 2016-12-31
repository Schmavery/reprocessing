open Reprocessing;

open P;

open PUtils;

type state = {squarePos: (int, int)};

let squareWidth = 300;

let squareHeight = 300;

let setup env => {
  size env 600 600;
  fill env (color 255 0 0);
  {squarePos: (0, 0)}
};

let draw state env => {
  background env (color 150 255 255);
  let (sx, sy) = state.squarePos;
  let (px, py) = pmouse env;
  let (x, y) as squarePos =
    if (mousePressed env && px > sx && px < sx + squareHeight && py > sy && py < sy + squareWidth) {
      let (mx, my) = mouse env;
      let dx = mx - px;
      let dy = my - py;
      (sx + dx, sy + dy)
    } else {
      state.squarePos
    };
  rect env x y squareWidth squareHeight;
  {...state, squarePos}
};

ReProcessor.run ::setup ::draw ();
