open Common;

open Glloader;

open Glhelpers;

open Utils;

open Drawfunctions;

module PUtils = PUtils;

module PConstants = PConstants;

module P = P;

type userCallbackT 'a = 'a => ref glState => ('a, glState);

let afterDraw f (env: ref glEnv) => {
  let rate = int_of_float (1000. /. f);
  env := {
    ...!env,
    mouse: {...(!env).mouse, prevPos: (!env).mouse.pos},
    frame: {count: (!env).frame.count + 1, rate}
  }
};

module ReProcessor: ReProcessorT = {
  type t = ref glEnv;
  let run ::setup ::draw=? ::mouseMove=? ::mouseDragged=? ::mouseDown=? ::mouseUp=? () => {
    Random.self_init ();
    PUtils.noiseSeed (Random.int (PUtils.pow 2 30 - 1));
    let env = ref (createCanvas (Gl.Window.init argv::Sys.argv) 200 200);
    let userState = ref (setup env);
    Gl.render
      window::(!env).window
      displayFunc::(
        switch draw {
        | Some draw => (
            fun f => {
              userState := draw !userState env;
              afterDraw f env
            }
          )
        | None => (fun f => ())
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
