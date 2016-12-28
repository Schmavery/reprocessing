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
  let (sx, sy) = state.squarePos;
  let (px, py) = (pmouseX env, pmouseY env);
  let (x, y) as squarePos =
    if (mousePressed env && px > sx && px < sx + squareHeight && py > sy && py < sy + squareWidth) {
      let (mx, my) = (mouseX env, mouseY env);
      let dx = mx - px;
      let dy = my - py;
      (sx + dx, sy + dy)
    } else {
      state.squarePos
    };
  clear env;
  rect env x y squareWidth squareHeight;
  {...state, squarePos}
};

ReProcessor.run ::setup ::draw ();
