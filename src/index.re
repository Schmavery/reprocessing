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
  stroke env (color 0 0 255);
  lineWeight env 10;
  line env (0, 0) (150, 150);
  lineWeight env 1;
  stroke env (color 0 255 0);
  line env (100, 100) (100, 200);
  {...state, squarePos}
};

ReProcessor.run ::setup ::draw ();
