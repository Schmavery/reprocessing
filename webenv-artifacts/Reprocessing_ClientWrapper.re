external removeFromDOM : 'a => unit = "remove" [@@bs.send];
external setID : 'a => string => unit = "id" [@@bs.set];
external appendChild : 'a => 'b => unit = "appendChild" [@@bs.send];
let init ::argv => {
  removeFromDOM (Document.getElementById "main-canvas");
  let canvas = createElement "canvas";
  setBackgroundColor (getStyle canvas) "black";
  setID canvas "main-canvas";
  appendChild (Document.getElementById "canvas-wrapper") canvas;
  canvas
};
