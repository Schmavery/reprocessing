# Reprocessing

Reprocessing is a high-level drawing library, inspired by [Processing](https://processing.org), allowing you to write code that'll run on the web (using WebGL) and natively (using OpenGL).

<!--!
```reason;shared(sandbox)
[@bs.val] external sandboxCanvasId: string = "";
[@bs.val] external sandboxCanvas: 'canvas = "";
[@bs.val] external containerDiv: 'node = "";
[@bs.send] external addEventListener: ('node, string, 'eventT => unit) => unit = "addEventListener";
let id = sandboxCanvasId;
addEventListener(containerDiv, "mouseleave", (_) => Reprocessing.playPause(id, false) |> ignore);
addEventListener(containerDiv, "mouseenter", (_) => Reprocessing.playPause(id, true) |> ignore);
Reprocessing.setScreenId(sandboxCanvasId);
```
-->

### Getting Started
```bash
npm install schmavery/reprocessing
```

### Example
```reason;use(sandbox);canvas
open Reprocessing;

let setup = (env) => {
  Env.size(~width=200, ~height=200, env);
};

let draw = (_state, env) => {
  Draw.background(Constants.black, env);
  Draw.fill(Constants.red, env);
  Draw.rect(~pos=(50, 50), ~width=100, ~height=100, env)
};

run(~setup, ~draw, ());
```

### Build
```sh
npm run build
```

This will draw a simple red square on a black background.  Compare this to [reglexampleproject](https://github.com/bsansouci/reasonglexampleproject/blob/simple/src/index.re), which takes 200+ lines to do the exact same thing.  This difference is even more notable on bigger projects.  Check out the code for a [draggable red square](https://github.com/Schmavery/reprocessing/blob/master/examples/redsquare.re).

## Projects using Reprocessing

- [Gravitron](https://github.com/jaredly/gravitron)
- [Oh No! Zombies!](https://github.com/bsansouci/ludum-dare-40)
- [2048](https://github.com/bsansouci/reprocessing-example/tree/2048)
- [FlappyBird](https://github.com/bsansouci/reprocessing-example/tree/livestream-flappybird)
- [Pong](https://github.com/illbexyz/repong)
- [Perlin Noise Flow Field](https://github.com/ekosz/reprocessing-example-flow-field)
- [L-System](https://github.com/Rigellute/L-system-reasonml)
