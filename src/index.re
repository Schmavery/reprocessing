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

let mouseDragged state env => {
  let {squarePos: (x, y)} = state;
  let (mouseX, mouseY) = (mouseX env, mouseY env);
  if (mouseX > x && mouseX < x + squareHeight && mouseY > y && mouseY < y + squareWidth) {
    let dx = mouseX - pmouseX env;
    let dy = mouseY - pmouseY env;
    {...state, squarePos: (x + dx, y + dy)}
  } else {
    state
  }
};

let draw state env => {
  let (x, y) = state.squarePos;
  clear env;
  rect env x y squareWidth squareHeight;
  state
};

ReProcessor.run ::setup ::draw ::mouseDragged ();
