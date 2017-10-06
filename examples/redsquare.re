open Reprocessing.Draw;

open Reprocessing.Env;

open Reprocessing.Utils;

type state = (int, int);

let squareSize = 300;

let setup env => {
  size width::600 height::600 env;
  fill Reprocessing.Constants.red env;
  (0, 0)
};

let draw squarePos env => {
  background (color r::150 g::255 b::255 a::255) env;
  let (sx, sy) = squarePos;
  let (px, py) = pmouse env;
  let (x, y) as squarePos =
    if (
      mousePressed env &&
      px > sx && px < sx + squareSize && py > sy && py < sy + squareSize
    ) {
      let (mx, my) = mouse env;
      let dx = mx - px;
      let dy = my - py;
      (sx + dx, sy + dy)
    } else {
      squarePos
    };
  rect pos::(x, y) width::squareSize height::squareSize env;
  squarePos
};

Reprocessing.run ::setup ::draw ();
