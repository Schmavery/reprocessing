open Reasongl;

open Reprocessing_Internal;

module Utils = Reprocessing_Utils;

module Constants = Reprocessing_Constants;

module Draw = Reprocessing_Draw;

module Env = Reprocessing_Env;

module Common = Reprocessing_Common;

module Events = Reprocessing_Events;

include Reprocessing_Types.Types;

let afterDraw f (env: Common.glEnv) => {
  open Common;
  let rate = int_of_float (1000. /. f);
  env.mouse.prevPos = env.mouse.pos;
  env.frame = {count: env.frame.count + 1, rate};
  Matrix.copyInto src::Matrix.identity dst::env.matrix;
  /* Flush the batching buffer at the end of every frame. */
  if (env.batch.elementPtr > 0) {
    flushGlobalBatch env
  }
};

let run
    ::setup
    ::draw=?
    ::mouseMove=?
    ::mouseDragged=?
    ::mouseDown=?
    ::mouseUp=?
    ::keyPressed=?
    ::keyReleased=?
    ::keyTyped=?
    () => {
  Random.self_init ();
  Reprocessing_Utils.noiseSeed (
    Random.int (Reprocessing_Utils.pow base::2 exp::(30 - 1))
  );
  let env =
    Reprocessing_Internal.createCanvas
      (Reprocessing_ClientWrapper.init argv::Sys.argv) 200 200;
  let userState = ref (setup env);

  /** This is a basically a hack to get around the default behavior of drawing something inside setup.
      Because OpenGL uses double buffering, drawing in setup will result in a flickering shape, as the data
      will only be in one buffer. To circumvent this we draw, read the pixel data and store that array, advance
      one frame and then paint that array as a texture on top before calling draw for the 2nd time. This ensures
      that both internal buffers contain the same data. **/
  let reDrawPreviousBufferOnSecondFrame = {
    open Common;
    let width = Gl.Window.getWidth env.window;
    let height = Gl.Window.getHeight env.window;
    let data = Gl.readPixels_RGBA context::env.gl x::0 y::0 ::width ::height;
    let textureBuffer = Gl.createTexture context::env.gl;
    Gl.bindTexture
      context::env.gl target::RGLConstants.texture_2d texture::textureBuffer;
    Gl.texImage2D_RGBA
      context::env.gl
      target::RGLConstants.texture_2d
      level::0
      ::width
      ::height
      border::0
      ::data;
    Gl.texParameteri
      context::env.gl
      target::RGLConstants.texture_2d
      pname::RGLConstants.texture_mag_filter
      param::RGLConstants.linear;
    Gl.texParameteri
      context::env.gl
      target::RGLConstants.texture_2d
      pname::RGLConstants.texture_min_filter
      param::RGLConstants.linear;
    Gl.texParameteri
      context::env.gl
      target::RGLConstants.texture_2d
      pname::RGLConstants.texture_wrap_s
      param::RGLConstants.clamp_to_edge;
    Gl.texParameteri
      context::env.gl
      target::RGLConstants.texture_2d
      pname::RGLConstants.texture_wrap_t
      param::RGLConstants.clamp_to_edge;
    fun () => {
      let (x, y) = (0, 0);
      let (x1, y1) = (float_of_int @@ x + width, float_of_int @@ y);
      let (x2, y2) = (float_of_int x, float_of_int @@ y);
      let (x3, y3) = (float_of_int @@ x + width, float_of_int @@ y + height);
      let (x4, y4) = (float_of_int x, float_of_int @@ y + height);
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
      drawGeometry
        vertexArray::(
          Gl.Bigarray.of_array Gl.Bigarray.Float32 verticesColorAndTexture
        )
        elementArray::(
          Gl.Bigarray.of_array Gl.Bigarray.Uint16 [|0, 1, 2, 1, 2, 3|]
        )
        mode::RGLConstants.triangles
        count::6
        ::textureBuffer
        env
    }
  };

  /** Start the render loop. **/
  Gl.render
    window::env.window
    displayFunc::(
      fun f => {
        if (env.frame.count === 2) {
          reDrawPreviousBufferOnSecondFrame ()
        };
        switch draw {
        | Some draw => userState := draw !userState env
        | None => ()
        };
        afterDraw f env
      }
    )
    mouseDown::(
      fun button::_ state::_ ::x ::y => {
        env.mouse.pos = (x, y);
        env.mouse.pressed = true;
        switch mouseDown {
        | Some mouseDown => userState := mouseDown !userState env
        | None => ()
        }
      }
    )
    mouseUp::(
      fun button::_ state::_ ::x ::y => {
        env.mouse.pos = (x, y);
        env.mouse.pressed = false;
        switch mouseUp {
        | Some mouseUp => userState := mouseUp !userState env
        | None => ()
        }
      }
    )
    mouseMove::(
      fun ::x ::y => {
        env.mouse.pos = (x, y);
        if env.mouse.pressed {
          switch mouseDragged {
          | Some mouseDragged => userState := mouseDragged !userState env
          | None => ()
          }
        } else {
          switch mouseMove {
          | Some mouseMove => userState := mouseMove !userState env
          | None => ()
          }
        }
      }
    )
    windowResize::(
      fun () =>
        if env.size.resizeable {
          let height = Gl.Window.getHeight env.window;
          let width = Gl.Window.getWidth env.window;
          resetSize env width height
        } else {
          Env.size width::(Env.width env) height::(Env.height env) env
        }
    )
    keyDown::(
      fun ::keycode ::repeat => {
        env.keyboard.keyCode = keycode;
        if (not repeat) {
          switch keyPressed {
          | Some keyPressed => userState := keyPressed !userState env
          | None => ()
          }
        };
        switch keyTyped {
        | Some keyTyped => userState := keyTyped !userState env
        | None => ()
        }
      }
    )
    keyUp::(
      fun ::keycode => {
        env.keyboard.keyCode = keycode;
        switch keyReleased {
        | Some keyReleased => userState := keyReleased !userState env
        | None => ()
        }
      }
    )
    ()
};
