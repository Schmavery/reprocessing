open Reasongl;

[@bs.send] external removeFromDOM : 'a => unit = "remove";

[@bs.set] external setID : ('a, string) => unit = "id";

[@bs.send] external appendChild : ('a, 'b) => unit = "appendChild";

let init = (~argv) => {
  removeFromDOM(Document.getElementById("main-canvas"));
  let canvas = createElement("canvas");
  setBackgroundColor(getStyle(canvas), "black");
  setID(canvas, "main-canvas");
  appendChild(Document.getElementById("canvas-wrapper"), canvas);
  canvas
};
