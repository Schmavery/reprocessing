open Reprocessing_Common;

module Matrix = Reprocessing_Matrix;

let width = (env) => env.size.width;

let height = (env) => env.size.height;

let windowWidth = (env) => Reasongl.Gl.Window.getWidth(env.window);

let windowHeight = (env) => Reasongl.Gl.Window.getHeight(env.window);

let mouse = (env) => env.mouse.pos;

let pmouse = (env) => env.mouse.prevPos;

let mousePressed = (env) => env.mouse.pressed;

let keyCode = (env) => env.keyboard.keyCode;

let key = (key, env) => Reprocessing_Common.KeySet.mem(key, env.keyboard.down);

let keyPressed = (key, env) =>
  Reprocessing_Common.KeySet.mem(key, env.keyboard.pressed);

let keyReleased = (key, env) =>
  Reprocessing_Common.KeySet.mem(key, env.keyboard.released);

let size = (~width, ~height, env: glEnv) => {
  Reasongl.Gl.Window.setWindowSize(~window=env.window, ~width, ~height);
  Reprocessing_Internal.resetSize(env, width, height)
};

let resizeable = (resizeable, env: glEnv) => env.size.resizeable = resizeable;

let frameRate = (env: glEnv) => env.frame.rate;

let frameCount = (env: glEnv) => env.frame.count;

let deltaTime = (env: glEnv) => env.frame.deltaTime;

let localizePointf = (p: (float, float), env: glEnv) =>
  Matrix.(matptmul(matinv(env.matrix), p));

let localizePoint = ((x, y): (int, int), env: glEnv) => {
  let (lx, ly) =
    Matrix.(matptmul(matinv(env.matrix), (float_of_int(x), float_of_int(y))));
  (int_of_float(lx), int_of_float(ly))
};
