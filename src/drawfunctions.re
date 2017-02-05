open Common;

open Glloader;

open Glhelpers;

open Utils;

open Font;

module P = {
  let width env => env.size.width;
  let height env => env.size.height;
  let mouse env => env.mouse.pos;
  let pmouse env => env.mouse.prevPos;
  let mousePressed env => env.mouse.pressed;
  let keyCode env => env.keyboard.keyCode;
  let translate dx dy env => Matrix.(matmatmul env.matrix (createTranslation dx dy));
  let rotate theta env => Matrix.(matmatmul env.matrix (createRotation theta));
  let fill (c: colorT) (env: glEnv) => env.style = {...env.style, fillColor: Some c};
  let noFill (env: glEnv) => env.style = {...env.style, fillColor: None};
  let stroke color env => env.style = {...env.style, strokeColor: Some color};
  let noStroke env => env.style = {...env.style, strokeColor: None};
  let strokeWeight weight env => env.style = {...env.style, strokeWeight: weight};
  let pushStyle env => env.styleStack = [env.style, ...env.styleStack];
  let popStyle env =>
    switch env.styleStack {
    /* Matches Processing error message */
    | [] => failwith "Too many `popStyle` without enough `pushStyle`."
    | [hd, ...tl] =>
      env.style = hd;
      env.styleStack = tl
    };
  let frameRate (env: glEnv) => env.frame.rate;
  let frameCount (env: glEnv) => env.frame.count;
  let size width height (env: glEnv) => {
    Gl.Window.setWindowSize window::env.window ::width ::height;
    resetSize env width height
  };
  let resizeable resizeable (env: glEnv) => env.size.resizeable = resizeable;
  let loadImage = loadImage;
  let image img x y (env: glEnv) =>
    switch !img {
    | None => print_endline "image not ready yet, just doing nothing :D"
    | Some ({width, height} as i) => drawImageInternal i x y 0 0 width height env
    };
  let clear env => Gl.clear env.gl (Constants.color_buffer_bit lor Constants.depth_buffer_bit);
  let linef p1 p2 (env: glEnv) =>
    switch env.style.strokeColor {
    | None => () /* don't draw stroke */
    | Some color =>
      let transform = Matrix.matptmul env.matrix;
      let ((xx1, yy1), (xx2, yy2)) = (transform p1, transform p2);
      let dx = xx2 -. xx1;
      let dy = yy2 -. yy1;
      let mag = PUtils.distf (xx1, yy1) (xx2, yy2);
      let radius = float_of_int env.style.strokeWeight /. 2.;
      let xthing = dy /. mag *. radius;
      let ything = -. dx /. mag *. radius;
      let x1 = xx2 +. xthing;
      let y1 = yy2 +. ything;
      let x2 = xx1 +. xthing;
      let y2 = yy1 +. ything;
      let x3 = xx2 -. xthing;
      let y3 = yy2 -. ything;
      let x4 = xx1 -. xthing;
      let y4 = yy1 -. ything;
      addRectToGlobalBatch env (x1, y1) (x2, y2) (x3, y3) (x4, y4) color
    };
  let line (x1, y1) (x2, y2) (env: glEnv) =>
    linef (float_of_int x1, float_of_int y1) (float_of_int x2, float_of_int y2) env;
  let ellipsef (center: (float, float)) (rx: float) (ry: float) (env: glEnv) => {
    switch env.style.fillColor {
    | None => () /* Don't draw fill */
    | Some fill => drawEllipseInternal env center rx ry env.matrix fill
    };
    switch env.style.strokeColor {
    | None => () /* Don't draw stroke */
    | Some stroke =>
      drawArcStroke
        env center rx ry 0. PConstants.tau false false env.matrix stroke env.style.strokeWeight
    }
  };
  let ellipse (cx: int, cy: int) rx ry (env: glEnv) =>
    ellipsef (float_of_int cx, float_of_int cy) (float_of_int rx) (float_of_int ry) env;
  let quadf p1 p2 p3 p4 (env: glEnv) => {
    let transform = Matrix.matptmul env.matrix;
    let (p1, p2, p3, p4) = (transform p1, transform p2, transform p3, transform p4);
    switch env.style.fillColor {
    | None => () /* Don't draw fill */
    | Some fill =>
      addRectToGlobalBatch env topLeft::p1 topRight::p2 bottomRight::p3 bottomLeft::p4 color::fill
    };
    switch env.style.strokeColor {
    | None => () /* don't draw stroke */
    | Some color =>
      linef p1 p2 env;
      linef p2 p3 env;
      linef p3 p4 env;
      linef p4 p1 env;
      let r = float_of_int env.style.strokeWeight /. 2.;
      let m = Matrix.identity;
      drawEllipseInternal env p1 r r m color;
      drawEllipseInternal env p2 r r m color;
      drawEllipseInternal env p3 r r m color;
      drawEllipseInternal env p4 r r m color
    }
  };
  let quad (x1, y1) (x2, y2) (x3, y3) (x4, y4) (env: glEnv) =>
    quadf
      (float_of_int x1, float_of_int y1)
      (float_of_int x2, float_of_int y2)
      (float_of_int x3, float_of_int y3)
      (float_of_int x4, float_of_int y4)
      env;
  let rectf x y width height (env: glEnv) =>
    quadf (x, y) (x +. width, y) (x +. width, y +. height) (x, y +. height) env;
  let rect x y width height (env: glEnv) =>
    rectf (float_of_int x) (float_of_int y) (float_of_int width) (float_of_int height) env;
  let pixelf (x: float) (y: float) color (env: glEnv) => {
    let w = float_of_int env.style.strokeWeight;
    addRectToGlobalBatch
      env
      bottomRight::(x +. w, y +. w)
      bottomLeft::(x, y +. w)
      topRight::(x +. w, y)
      topLeft::(x, y)
      ::color
  };
  let pixel x y color (env: glEnv) => pixelf (float_of_int x) (float_of_int y) color env;
  let trianglef p1 p2 p3 (env: glEnv) => {
    let transform = Matrix.matptmul env.matrix;
    let (p1, p2, p3) = (transform p1, transform p2, transform p3);
    switch env.style.fillColor {
    | None => () /* don't draw fill */
    | Some color => drawTriangleInternal env p1 p2 p3 ::color
    };
    switch env.style.strokeColor {
    | None => () /* don't draw stroke */
    | Some color =>
      linef p1 p2 env;
      linef p2 p3 env;
      linef p3 p1 env;
      let r = float_of_int env.style.strokeWeight /. 2.;
      let m = Matrix.identity;
      drawEllipseInternal env p1 r r m color;
      drawEllipseInternal env p2 r r m color;
      drawEllipseInternal env p3 r r m color
    }
  };
  let triangle (x1, y1) (x2, y2) (x3, y3) (env: glEnv) =>
    trianglef
      (float_of_int x1, float_of_int y1)
      (float_of_int x2, float_of_int y2)
      (float_of_int x3, float_of_int y3)
      env;
  let arcf centerPt rx ry start stop isOpen isPie (env: glEnv) => {
    switch env.style.fillColor {
    | None => () /* don't draw fill */
    | Some color => drawArcInternal env centerPt rx ry start stop isPie env.matrix color
    };
    switch env.style.strokeColor {
    | None => () /* don't draw stroke */
    | Some stroke =>
      drawArcStroke
        env centerPt rx ry start stop isOpen isPie env.matrix stroke env.style.strokeWeight
    }
  };
  let arc (cx, cy) rx ry start stop isOpen isPie (env: glEnv) =>
    arcf
      (float_of_int cx, float_of_int cy)
      (float_of_int rx)
      (float_of_int ry)
      start
      stop
      isOpen
      isPie
      env;
  let loadFont filename (env: glEnv) => Font.parseFontFormat env filename;
  let text fnt str x y (env: glEnv) => Font.drawString env fnt str x y;
  let background color (env: glEnv) => {
    let w = float_of_int (width env);
    let h = float_of_int (height env);
    addRectToGlobalBatch
      env bottomRight::(w, h) bottomLeft::(0., h) topRight::(w, 0.) topLeft::(0., 0.) ::color
  };
};
