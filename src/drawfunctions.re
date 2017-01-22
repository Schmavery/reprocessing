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
  let fill (env: ref glEnv) (c: colorT) => env := {...!env, currFill: Some c};
  let noFill (env: ref glEnv) => env := {...!env, currFill: None};
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
  let stroke env color => env := {...!env, stroke: {...(!env).stroke, color}};
  let strokeWeight env weight => env := {...!env, stroke: {...(!env).stroke, weight}};
  let linef env (xx1: float, yy1: float) (xx2: float, yy2: float) => {
    let dx = xx2 -. xx1;
    let dy = yy2 -. yy1;
    let mag = PUtils.distf (xx1, yy1) (xx2, yy2);
    let radius = float_of_int (!env).stroke.weight /. 2.;
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
    addRectToGlobalBatch env (x1, y1) (x2, y2) (x3, y3) (x4, y4) (!env).stroke.color
  };
  let line env (x1, y1) (x2, y2) =>
    linef env (float_of_int x1, float_of_int y1) (float_of_int x2, float_of_int y2);
  let ellipsef env (a: float) (b: float) (c: float) (d: float) =>
    switch (!env).currFill {
    | Some fill => drawEllipseInternal env a b c d fill
    | None => () /* Don't draw fill */
    };
  let ellipse env a b c d =>
    ellipsef env (float_of_int a) (float_of_int b) (float_of_int c) (float_of_int d);
  let rectf (env: ref glEnv) x y width height => {
    switch (!env).currFill {
    | Some fill =>
      addRectToGlobalBatch
        env (x +. width, y +. height) (x, y +. height) (x +. width, y) (x, y) fill
    | None => () /* Don't draw fill */
    };
    linef env (x +. width, y) (x, y);
    linef env (x +. width, y) (x +. width, y +. height);
    linef env (x +. width, y +. height) (x, y +. width);
    linef env (x, y +. height) (x, y);
    let r = float_of_int (!env).stroke.weight /. 2.;
    let c = (!env).stroke.color;
    /* drawEllipseInternal env x y r r c;
    drawEllipseInternal env (x +. width) y r r c;
    drawEllipseInternal env (x +. width) (y +. height) r r c;
    drawEllipseInternal env x (y +. height) r r c */
  };
  let rect (env: ref glEnv) x y width height =>
    rectf env (float_of_int x) (float_of_int y) (float_of_int width) (float_of_int height);
  let pixel env x y color =>
    addRectToGlobalBatch
      env
      (float_of_int @@ x + (!env).stroke.weight, float_of_int @@ y + (!env).stroke.weight)
      (float_of_int x, float_of_int @@ y + (!env).stroke.weight)
      (float_of_int @@ x + (!env).stroke.weight, float_of_int y)
      (float_of_int x, float_of_int y)
      color;
  let loadFont env filename => Font.parseFontFormat env filename;
  let text env fnt str x y => Font.drawString env fnt str x y;
  let background env color => {
    let w = float_of_int (width env);
    let h = float_of_int (height env);
    addRectToGlobalBatch env (w, h) (0., h) (w, 0.) (0., 0.) color
  };
};
