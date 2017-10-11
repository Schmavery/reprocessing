open Reprocessing_Common;

let width env => env.size.width;

let height env => env.size.height;

let mouse env => env.mouse.pos;

let pmouse env => env.mouse.prevPos;

let mousePressed env => env.mouse.pressed;

let keyCode env => env.keyboard.keyCode;

let size ::width ::height (env: glEnv) => {
  Reasongl.Gl.Window.setWindowSize window::env.window ::width ::height;
  Reprocessing_Internal.resetSize env width height
};

let resizeable resizeable (env: glEnv) => env.size.resizeable = resizeable;

let frameRate (env: glEnv) => env.frame.rate;

let frameCount (env: glEnv) => env.frame.count;

let deltaTime (env: glEnv) => env.frame.deltaTime;
