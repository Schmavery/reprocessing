open Reasongl;

open Reprocessing_Internal;

module Utils = Reprocessing_Utils;

module Constants = Reprocessing_Constants;

module Draw = Reprocessing_Draw;

module Env = Reprocessing_Env;

module Common = Reprocessing_Common;

module Events = Reprocessing_Events;

include Reprocessing_Types.Types;

type hotreloadT('a) = {
  mutable started: bool,
  mutable filename: string,
  mutable draw: ('a, Reprocessing_Common.glEnv) => 'a,
  mutable mouseMove: ('a, Reprocessing_Common.glEnv) => 'a,
  mutable mouseDragged: ('a, Reprocessing_Common.glEnv) => 'a,
  mutable mouseDown: ('a, Reprocessing_Common.glEnv) => 'a,
  mutable mouseUp: ('a, Reprocessing_Common.glEnv) => 'a,
  mutable keyPressed: ('a, Reprocessing_Common.glEnv) => 'a,
  mutable keyReleased: ('a, Reprocessing_Common.glEnv) => 'a,
  mutable keyTyped: ('a, Reprocessing_Common.glEnv) => 'a
};

let hotreloadData = Obj.magic(ref(None));

let afterDraw = (f, env: Common.glEnv) => {
  open Common;
  let rate = int_of_float(1000. /. f);
  env.mouse.prevPos = env.mouse.pos;
  env.frame = {count: env.frame.count + 1, rate, deltaTime: f /. 1000.};
  env.keyboard.released = Common.KeySet.empty;
  env.keyboard.pressed = Common.KeySet.empty;
  Matrix.copyInto(~src=Matrix.identity, ~dst=env.matrix);
  /* Flush the batching buffer at the end of every frame. */
  if (env.batch.elementPtr > 0) {
    flushGlobalBatch(env)
  }
};

let unwrapOrDefault = (default, opt) =>
  switch opt {
  | Some(a) => a
  | None => default
  };

let identity = (a, _) => a;

let hotreload = (filename) => {
  /* ... */
  let _a = Reprocessing_Draw.translate;
  let _b = Reprocessing_Utils.color;
  let _c = Reprocessing_Constants.red;
  let _d = Reprocessing_Common.circularBufferSize;
  let _e = Reprocessing_Events.keycodeMap;
  hotreloadData :=
    Some({
      started: false,
      filename,
      draw: identity,
      keyPressed: identity,
      keyReleased: identity,
      keyTyped: identity,
      mouseMove: identity,
      mouseDragged: identity,
      mouseDown: identity,
      mouseUp: identity
    });
  Reprocessing_Hotreload.checkRebuild(true, filename)
};

let run =
    (
      ~setup,
      ~draw=?,
      ~mouseMove=?,
      ~mouseDragged=?,
      ~mouseDown=?,
      ~mouseUp=?,
      ~keyPressed=?,
      ~keyReleased=?,
      ~keyTyped=?,
      ()
    ) => {
  let unwrap = unwrapOrDefault(identity);
  let fns =
    switch hotreloadData^ {
    | None => {
        started: false,
        filename: "",
        draw: unwrap(draw),
        keyPressed: unwrap(keyPressed),
        keyReleased: unwrap(keyReleased),
        keyTyped: unwrap(keyTyped),
        mouseMove: unwrap(mouseMove),
        mouseDragged: unwrap(mouseDragged),
        mouseDown: unwrap(mouseDown),
        mouseUp: unwrap(mouseUp)
      }
    | Some(hr) =>
      hr.draw = unwrap(draw);
      hr.keyPressed = unwrap(keyPressed);
      hr.keyReleased = unwrap(keyReleased);
      hr.keyTyped = unwrap(keyTyped);
      hr.mouseMove = unwrap(mouseMove);
      hr.mouseDragged = unwrap(mouseDragged);
      hr.mouseDown = unwrap(mouseDown);
      hr.mouseUp = unwrap(mouseUp);
      print_endline("Succesfully changed functions");
      hr
    };
  if (! fns.started) {
    fns.started = true;
    Random.self_init();
    Reprocessing_Utils.noiseSeed(Random.int(Reprocessing_Utils.pow(~base=2, ~exp=30 - 1)));
    let env =
      Reprocessing_Internal.createCanvas(
        Reprocessing_ClientWrapper.init(~argv=Sys.argv),
        200,
        200
      );
    Reprocessing_Font.Font.loadDefaultFont(env);
    let userState = ref(setup(env));

    /*** This is a basically a hack to get around the default behavior of drawing something inside setup.
         Because OpenGL uses double buffering, drawing in setup will result in a flickering shape, as the data
         will only be in one buffer. To circumvent this we draw, read the pixel data and store that array, advance
         one frame and then paint that array as a texture on top before calling draw for the 2nd time. This ensures
         that both internal buffers contain the same data. **/
    let reDrawPreviousBufferOnSecondFrame = {
      open Common;
      let width = Gl.Window.getWidth(env.window);
      let height = Gl.Window.getHeight(env.window);
      let data = Gl.readPixels_RGBA(~context=env.gl, ~x=0, ~y=0, ~width, ~height);
      let textureBuffer = Gl.createTexture(~context=env.gl);
      Gl.bindTexture(~context=env.gl, ~target=RGLConstants.texture_2d, ~texture=textureBuffer);
      Gl.texImage2D_RGBA(
        ~context=env.gl,
        ~target=RGLConstants.texture_2d,
        ~level=0,
        ~width,
        ~height,
        ~border=0,
        ~data
      );
      Gl.texParameteri(
        ~context=env.gl,
        ~target=RGLConstants.texture_2d,
        ~pname=RGLConstants.texture_mag_filter,
        ~param=RGLConstants.linear
      );
      Gl.texParameteri(
        ~context=env.gl,
        ~target=RGLConstants.texture_2d,
        ~pname=RGLConstants.texture_min_filter,
        ~param=RGLConstants.linear
      );
      Gl.texParameteri(
        ~context=env.gl,
        ~target=RGLConstants.texture_2d,
        ~pname=RGLConstants.texture_wrap_s,
        ~param=RGLConstants.clamp_to_edge
      );
      Gl.texParameteri(
        ~context=env.gl,
        ~target=RGLConstants.texture_2d,
        ~pname=RGLConstants.texture_wrap_t,
        ~param=RGLConstants.clamp_to_edge
      );
      () => {
        let (x, y) = (0, 0);
        let (x1, y1) = (float_of_int @@ x + width, float_of_int @@ y);
        let (x2, y2) = (float_of_int(x), float_of_int @@ y);
        let (x3, y3) = (float_of_int @@ x + width, float_of_int @@ y + height);
        let (x4, y4) = (float_of_int(x), float_of_int @@ y + height);
        let verticesColorAndTexture = [|
          x1,
          y1,
          0.0,
          0.0,
          0.0,
          0.0,
          1.,
          1.0,
          1.0,
          x2,
          y2,
          0.0,
          0.0,
          0.0,
          0.0,
          1.,
          0.0,
          1.0,
          x3,
          y3,
          0.0,
          0.0,
          0.0,
          0.0,
          1.,
          1.0,
          0.0,
          x4,
          y4,
          0.0,
          0.0,
          0.0,
          0.0,
          1.,
          0.0,
          0.0
        |];
        drawGeometry(
          ~vertexArray=Gl.Bigarray.of_array(Gl.Bigarray.Float32, verticesColorAndTexture),
          ~elementArray=Gl.Bigarray.of_array(Gl.Bigarray.Uint16, [|0, 1, 2, 1, 2, 3|]),
          ~mode=RGLConstants.triangles,
          ~count=6,
          ~textureBuffer,
          env
        )
      }
    };

    /*** Start the render loop. **/
    Gl.render(
      ~window=env.window,
      ~displayFunc=
        (f) => {
          if (env.frame.count === 2) {
            reDrawPreviousBufferOnSecondFrame()
          };
          switch hotreloadData^ {
          | Some(_) => ignore @@ Reprocessing_Hotreload.checkRebuild(false, fns.filename)
          | None => ()
          };
          userState := fns.draw(userState^, env);
          afterDraw(f, env)
        },
      ~mouseDown=
        (~button as _, ~state as _, ~x, ~y) => {
          env.mouse.pos = (x, y);
          env.mouse.pressed = true;
          userState := fns.mouseDown(userState^, env)
        },
      ~mouseUp=
        (~button as _, ~state as _, ~x, ~y) => {
          env.mouse.pos = (x, y);
          env.mouse.pressed = false;
          userState := fns.mouseUp(userState^, env)
        },
      ~mouseMove=
        (~x, ~y) => {
          env.mouse.pos = (x, y);
          if (env.mouse.pressed) {
            userState := fns.mouseDragged(userState^, env)
          } else {
            userState := fns.mouseMove(userState^, env)
          }
        },
      ~windowResize=
        () =>
          if (env.size.resizeable) {
            let height = Gl.Window.getHeight(env.window);
            let width = Gl.Window.getWidth(env.window);
            resetSize(env, width, height)
          } else {
            Env.size(~width=Env.width(env), ~height=Env.height(env), env)
          },
      ~keyDown=
        (~keycode, ~repeat) => {
          env.keyboard.keyCode = keycode;
          if (! repeat) {
            userState := fns.keyPressed(userState^, env);
            env.keyboard.pressed = Common.KeySet.add(keycode, env.keyboard.pressed);
            env.keyboard.down = Common.KeySet.add(keycode, env.keyboard.down)
          };
          userState := fns.keyTyped(userState^, env)
        },
      ~keyUp=
        (~keycode) => {
          env.keyboard.keyCode = keycode;
          env.keyboard.released = Common.KeySet.add(keycode, env.keyboard.released);
          env.keyboard.down = Common.KeySet.remove(keycode, env.keyboard.down);
          userState := fns.keyReleased(userState^, env)
        },
      ()
    )
  }
};
