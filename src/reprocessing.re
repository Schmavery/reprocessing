/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
open Common;

open Glloader;

open Glhelpers;

open Utils;

module PUtils = PUtils;

module PConstants = PConstants;

module P = Drawfunctions.P;

type userCallbackT 'a = 'a => ref glState => ('a, glState);

let afterDraw f (env: ref glEnv) => {
  let rate = int_of_float (1000. /. f);
  env := {
    ...!env,
    mouse: {...(!env).mouse, prevPos: (!env).mouse.pos},
    frame: {count: (!env).frame.count + 1, rate}
  };
  /* Flush the batching buffer at the end of every frame. */
  flushGlobalBatch env
};

module ReProcessor: ReProcessorT = {
  type t = ref glEnv;
  let run ::setup ::draw=? ::mouseMove=? ::mouseDragged=? ::mouseDown=? ::mouseUp=? () => {
    Random.self_init ();
    PUtils.noiseSeed (Random.int (PUtils.pow 2 30 - 1));
    let env = ref (createCanvas (Gl.Window.init argv::Sys.argv) 200 200);
    let userState = ref (setup env);

    /** This is a basically a hack to get around the default behavior of drawing something inside setup.
        Because OpenGL uses double buffering, drawing in setup will result in a flickering shape, as the data
        will only be in one buffer. To circumvent this we draw, read the pixel data and store that array, advance
        one frame and then paint that array as a texture on top before calling draw for the 2nd time. This ensures
        that both internal buffers contain the same data. **/
    let reDrawPreviousBufferOnSecondFrame = {
      let width = Gl.Window.getWidth (!env).window;
      let height = Gl.Window.getHeight (!env).window;
      let data = Gl.readPixelsRGBA context::(!env).gl x::0 y::0 ::width ::height;
      let textureBuffer = Gl.createTexture context::(!env).gl;
      Gl.bindTexture context::(!env).gl target::Constants.texture_2d texture::textureBuffer;
      Gl.texImage2D
        context::(!env).gl
        target::Constants.texture_2d
        level::0
        internalFormat::Constants.rgba
        ::width
        ::height
        format::Constants.rgba
        type_::Constants.unsigned_byte
        ::data;
      Gl.texParameteri
        context::(!env).gl
        target::Constants.texture_2d
        pname::Constants.texture_mag_filter
        param::Constants.linear;
      Gl.texParameteri
        context::(!env).gl
        target::Constants.texture_2d
        pname::Constants.texture_min_filter
        param::Constants.linear;
      Gl.texParameteri
        context::(!env).gl
        target::Constants.texture_2d
        pname::Constants.texture_wrap_s
        param::Constants.clamp_to_edge;
      Gl.texParameteri
        context::(!env).gl
        target::Constants.texture_2d
        pname::Constants.texture_wrap_t
        param::Constants.clamp_to_edge;
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
          vertexArray::(Gl.Bigarray.of_array Gl.Bigarray.Float32 verticesColorAndTexture)
          elementArray::(Gl.Bigarray.of_array Gl.Bigarray.Uint16 [|0, 1, 2, 1, 2, 3|])
          mode::Constants.triangles
          count::6
          textureFlag::1.0
          ::textureBuffer
          !env
      }
    };

    /** Start the render loop. **/
    Gl.render
      window::(!env).window
      displayFunc::(
        fun f => {
          if ((!env).frame.count === 2) {
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
        fun ::button ::state ::x ::y => {
          env := {...!env, mouse: {...(!env).mouse, pos: (x, y), pressed: true}};
          switch mouseDown {
          | Some mouseDown => userState := mouseDown !userState env
          | None => ()
          }
        }
      )
      mouseUp::(
        fun ::button ::state ::x ::y => {
          env := {...!env, mouse: {...(!env).mouse, pos: (x, y), pressed: false}};
          switch mouseUp {
          | Some mouseUp => userState := mouseUp !userState env
          | None => ()
          }
        }
      )
      mouseMove::(
        fun ::x ::y => {
          env := {...!env, mouse: {...(!env).mouse, pos: (x, y)}};
          if (!env).mouse.pressed {
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
          if (!env).size.resizeable {
            let height = Gl.Window.getHeight (!env).window;
            let width = Gl.Window.getWidth (!env).window;
            resetSize env width height
          } else {
            P.size env (P.width env) (P.height env)
          }
      )
      ()
  };
};
