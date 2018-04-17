# Reprocessing

[![Build Status](https://travis-ci.org/Schmavery/reprocessing.svg?branch=master)](https://travis-ci.org/Schmavery/reprocessing)
[![Build status](https://ci.appveyor.com/api/projects/status/jgaaw641624db0pq/branch/master?svg=true&passingText=windows%20-%20OK&failingText=windows%20-%20Failing)](https://ci.appveyor.com/project/Schmavery/reprocessing/branch/master)



This is a high-level drawing library, inspired by [Processing](https://processing.org), allowing you to write code that'll run on the web (using WebGL) and natively (using OpenGL).


## Example
The [web environment](https://schmavery.github.io/reprocessing/) is the simplest way to try reprocessing. (It uses an older verison of the Reason syntax though, we're working on fixing that).

The 2nd simplest way to try is to clone [reprocessing-example](https://github.com/bsansouci/reprocessing-example).

See [below](#projects-using-reprocessing) for projects using Reprocessing!

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

## Getting Started
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

## Demo
There are a couple demos inside `examples`. Run `npm i` to install all deps and `npm run build` to build to JS (default). Open `index.html` in safari (or use `python -m SimpleHTTPServer 8000` to spawn a static server and go to `localhost:8000` in chrome).

Run `npm run build:bytecode` to build to a bytecode executable and run `./lib/bs/bytecode/index.byte`.

Run `npm run build:native` to build to a native executable and run `./lib/bs/native/index.native`.

See also [FlappyBird](https://github.com/Schmavery/FlappyBird) or [2048](https://github.com/bsansouci/reprocessing-example/tree/2048) for slightly bigger examples.


# Some Differences from Processing
- There is no magic - everything is proper Reason code.  This means that you have to call `Reprocessing.run` with the functions that you want to use.  You also have a couple of options about which utility modules to open.  See the `examples` directory for some different ways to do this.  It is recommended to `open Reprocessing` at the top, and then you can optionally open `Draw`, `Env` and `Utils` to make it look more like Processing code. Alternatively, they can be used directly, as can be seen above.

- For state management, we encourage the use of the `state` value that Reprocessing manages for the user.  To use this, decide on a datatype representing the state and return the initial value from `setup`.  This will be persisted behind the scenes and passed to every callback (such as `draw` and `mouseDown`).  Each callback should return the new value of the state (or the old value if it doesn't change).

- There are no built-in variables like `width` and `mouseX`.  Instead, these are functions that are called, passing in an environment object that is always provided.
```reason;prefix(1);no-run
open Reprocessing;
let draw = (state, env) => {
  let w = Env.width(env);
  print_endline("The current width is:" ++ string_of_int(w))
};
```

- The builtin `map` function is called `remap` instead to avoid confusion with the well-known `List.map` function which maps over a list of values. As, according to the Processing docs, this function "Re-maps a number from one range to another.", this naming seems appropriate.

- Because of the limitations of Reason, several utility functions that would otherwise accept either an integer or a float now expose a version with an `f` suffix, which supports floats.  Ex: `random` vs `randomf`.

- Points are expressed as tuples.  Instead of exposing a `mouseX` and `mouseY`, there is a `mouse`, which is a tuple of x and y values.

```reason;prefix(1);no-run
open Reprocessing;
let draw = (state, env) => {
  let (x, y) = Env.mouse(env);
  print_endline("The current mouse position is:" ++ (string_of_int(x) ++ string_of_int(y)))
};
```


# Using Fonts
The story for using fonts in your Reprocessing app is still under some development to make it nicer.  Right now we have support for writing text in a font defined in the [Angel Code font](http://www.angelcode.com/products/bmfont/) format. This is basically a bitmap of packed glyph textures along with a text file that describes it.

Check out [font-generator](https://github.com/bsansouci/font-generator) for a tool that can take any truetype or opentype font and output font files that Reprocessing can use.

The assets folder of this repo also has an [example](https://github.com/Schmavery/reprocessing/tree/master/assets/font) of a font that can be copied to your project and used.  In order to use a font once you have the files:
```prefix(2);suffix(1);no-run
open Reprocessing;
let fn = (filename, env) => {
let font = Draw.loadFont(~filename, env);
Draw.text(~font, ~body="Test!!!", ~pos=(10, 10), env);
}
```

## Projects using Reprocessing

- [Gravitron](https://github.com/jaredly/gravitron)
- [Oh No! Zombies!](https://github.com/bsansouci/ludum-dare-40)
- [2048](https://github.com/bsansouci/reprocessing-example/tree/2048)
- [FlappyBird](https://github.com/bsansouci/reprocessing-example/tree/livestream-flappybird)
- [Pong](https://github.com/illbexyz/repong)
- [Perlin Noise Flow Field](https://github.com/ekosz/reprocessing-example-flow-field)
- [L-System](https://github.com/Rigellute/L-system-reasonml)
