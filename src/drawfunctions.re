/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
open Common;

open Glloader;

open Glhelpers;

open Utils;

open Font;

module P = {
  let width env => (!env).size.width;
  let height env => (!env).size.height;
  let mouse env => (!env).mouse.pos;
  let pmouse env => (!env).mouse.prevPos;
  let mousePressed env => (!env).mouse.pressed;
  let fill (env: ref glEnv) (c: colorT) => (!env).style = {...(!env).style, fillColor: Some c};
  let noFill (env: ref glEnv) => (!env).style = {...(!env).style, fillColor: None};
  let stroke env color => (!env).style = {...(!env).style, strokeColor: Some color};
  let noStroke env color => (!env).style = {...(!env).style, strokeColor: None};
  let strokeWeight env weight => (!env).style = {...(!env).style, strokeWeight: weight};
  let pushStyle env => (!env).styleStack = [(!env).style, ...(!env).styleStack];
  let popStyle env =>
    switch (!env).styleStack {
    /* Matches Processing error message */
    | [] => failwith "Too many `popStyle` without enough `pushStyle`."
    | [hd, ...tl] =>
      (!env).style = hd;
      (!env).styleStack = tl
    };
  let frameRate (env: ref glEnv) => (!env).frame.rate;
  let frameCount (env: ref glEnv) => (!env).frame.count;
  let size (env: ref glEnv) width height => {
    Gl.Window.setWindowSize window::(!env).window ::width ::height;
    resetSize env width height
  };
  let resizeable (env: ref glEnv) resizeable =>
    env := {...!env, size: {...(!env).size, resizeable}};
  let loadImage = loadImage;
  let image (env: ref glEnv) img x y =>
    switch !img {
    | None => print_endline "image not ready yet, just doing nothing :D"
    | Some ({width, height} as i) => drawImageInternal i x y 0 0 width height env
    };
  let clear env => Gl.clear (!env).gl (Constants.color_buffer_bit lor Constants.depth_buffer_bit);
  let linef env (xx1: float, yy1: float) (xx2: float, yy2: float) =>
    switch (!env).style.strokeColor {
    | None => () /* don't draw stroke */
    | Some color =>
      let dx = xx2 -. xx1;
      let dy = yy2 -. yy1;
      let mag = PUtils.distf (xx1, yy1) (xx2, yy2);
      let radius = float_of_int (!env).style.strokeWeight /. 2.;
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
  let line env (x1, y1) (x2, y2) =>
    linef env (float_of_int x1, float_of_int y1) (float_of_int x2, float_of_int y2);
  let ellipsef env (center: (float, float)) (rx: float) (ry: float) =>
    switch (!env).style.fillColor {
    | None => () /* Don't draw fill */
    | Some fill => drawEllipseInternal env center rx ry fill
    };
  let ellipse env (cx: int, cy: int) rx ry =>
    ellipsef env (float_of_int cx, float_of_int cy) (float_of_int rx) (float_of_int ry);
  let quadf (env: ref glEnv) p1 p2 p3 p4 => {
    switch (!env).style.fillColor {
    | None => () /* Don't draw fill */
    | Some fill =>
      addRectToGlobalBatch env topLeft::p1 topRight::p2 bottomRight::p3 bottomLeft::p4 color::fill
    };
    switch (!env).style.strokeColor {
    | None => () /* don't draw stroke */
    | Some color =>
      linef env p1 p2;
      linef env p2 p3;
      linef env p3 p4;
      linef env p4 p1;
      let r = float_of_int (!env).style.strokeWeight /. 2.;
      drawEllipseInternal env p1 r r color;
      drawEllipseInternal env p2 r r color;
      drawEllipseInternal env p3 r r color;
      drawEllipseInternal env p4 r r color
    }
  };
  let quad (env: ref glEnv) (x1, y1) (x2, y2) (x3, y3) (x4, y4) =>
    quadf
      env
      (float_of_int x1, float_of_int y1)
      (float_of_int x2, float_of_int y2)
      (float_of_int x3, float_of_int y3)
      (float_of_int x4, float_of_int y4);
  let rectf (env: ref glEnv) x y width height =>
    quadf env (x, y) (x +. width, y) (x +. width, y +. height) (x, y +. height);
  let rect (env: ref glEnv) x y width height =>
    rectf env (float_of_int x) (float_of_int y) (float_of_int width) (float_of_int height);
  let pixelf env (x: float) (y: float) color => {
    let w = float_of_int (!env).style.strokeWeight;
    addRectToGlobalBatch
      env
      bottomRight::(x +. w, y +. w)
      bottomLeft::(x, y +. w)
      topRight::(x +. w, y)
      topLeft::(x, y)
      ::color
  };
  let pixel env x y color => pixelf env (float_of_int x) (float_of_int y) color;
  let trianglef env p1 p2 p3 => {
    switch (!env).style.fillColor {
    | None => () /* don't draw fill */
    | Some color => drawTriangleInternal env p1 p2 p3 ::color
    };
    switch (!env).style.strokeColor {
    | None => () /* don't draw stroke */
    | Some color =>
      linef env p1 p2;
      linef env p2 p3;
      linef env p3 p1;
      let r = float_of_int (!env).style.strokeWeight /. 2.;
      drawEllipseInternal env p1 r r color;
      drawEllipseInternal env p2 r r color;
      drawEllipseInternal env p3 r r color
    }
  };
  let triangle env (x1, y1) (x2, y2) (x3, y3) =>
    trianglef
      env
      (float_of_int x1, float_of_int y1)
      (float_of_int x2, float_of_int y2)
      (float_of_int x3, float_of_int y3);
  let arcf env (cx, cy) rx ry start stop =>
    switch (!env).style.fillColor {
    | None => () /* don't draw fill */
    | Some color => drawArcInternal env (cx, cy) rx ry start stop color
    };
  let loadFont env filename => Font.parseFontFormat env filename;
  let text env fnt str x y => Font.drawString env fnt str x y;
  let background env color => {
    let w = float_of_int (width env);
    let h = float_of_int (height env);
    addRectToGlobalBatch
      env bottomRight::(w, h) bottomLeft::(0., h) topRight::(w, 0.) topLeft::(0., 0.) ::color
  };
};
