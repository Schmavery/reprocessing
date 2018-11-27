# Reprocessing

[![Build Status](https://travis-ci.org/Schmavery/reprocessing.svg?branch=master)](https://travis-ci.org/Schmavery/reprocessing)
[![Build status](https://ci.appveyor.com/api/projects/status/jgaaw641624db0pq/branch/master?svg=true&passingText=windows%20-%20OK&failingText=windows%20-%20Failing)](https://ci.appveyor.com/project/Schmavery/reprocessing/branch/master)



This is a high-level drawing library, inspired by [Processing](https://processing.org), allowing you to write code that'll run on the web (using WebGL) and natively (using OpenGL).


## Example
The [interactive docs](https://schmavery.github.io/reprocessing/) are the simplest way to try reprocessing. (They are generated using [redoc](https://github.com/jaredly/redoc)!).

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
npm install reprocessing
```

> Note: on linux, you may need to also install some libraries):<br>
`apt install libsdl2-dev fcitx-libs-dev libibus-1.0-dev`

### Code Example
_Clone [reprocessing-example](https://github.com/bsansouci/reprocessing-example) and follow instructions there to setup a new project._

```reason ;use(sandbox);canvas
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

See [below](#projects-using-reprocessing) for examples!

# FAQs
### Where do I find `x` function?
There are a few modules in `Reprocessing` that store most functions you will want to use.
The best way to find one is to use the *search* on the docs site: https://schmavery.github.io/reprocessing/

In general:
- `Draw` contains functions that draw to the screen (and affect drawing), like `rect` and `image`.
- `Env` contains functions involving the environment (or window) you are running in. For example, `mouse` and `size`.
- `Utils` contains many static helper functions from Processing such as `lerp` and `dist`.
- `Constants` contains some mathematical and color-related constants, like `pi` and `green`.

### Why do some functions have an `"f"` at the end?
Several utility functions that would otherwise accept either an integer or a float in Processing expose a version with an `f` suffix, which supports floats.  Ex: `random` vs `randomf`. This lets you use whichever you prefer without needing to convert all the time.

### When do I run loadImage or loadFont?
It is best to run these functions in the setup function. They are fairly expensive to run and setup is usually the easiest place to load your assets once. Then you can keep a reference to them in your state and draw them as many times as you want!

### How do I use different fonts when drawing text?
There is a default font in Reprocessing that will be automatically used if you use `Draw.text` without providing a font. However, you frequently want to have your own font!

The story for using fonts in your Reprocessing app is still under some development to make it nicer.  Right now we have support for writing text in a font defined in the [Angel Code font](http://www.angelcode.com/products/bmfont/) format. This is basically a bitmap of packed glyph textures along with a text file that describes it.

★★★ Check out [font-generator](https://github.com/bsansouci/font-generator) for a tool that can take any truetype or opentype font and output font files that Reprocessing can use.

In order to use a font once you have the files:
```reason ;prefix(2);suffix(1);no-run
let font = Draw.loadFont(~filename, env);
Draw.text(~font, ~body="Test!!!", ~pos=(10, 10), env);
```

### Why is there no support for 3D drawing?
The original goal for reprocessing was to make something extremely easy to use and build real (2d) games and experiences with in ReasonML. Processing's 2D API does an amazing job at making graphics approachable. It would be really neat to be able to extend this to 3D creations but I do tend to feel that the 3D API is significantly more complex in some ways. It adds several new concepts such as 3d shapes, texture/materials/lighting, and we'd need to extend several functions to optionally support a third dimension. It also doesn't let you avoid the matrix functions which can be counterintuitive and camera logic gets more involved. We may consider trying to add support in the future but it currently isn't on the roadmap.

# Some Differences from Processing
- For state management, we encourage the use of the `state` value that Reprocessing manages for the user.  To use this, decide on a datatype representing the state and return the initial value from `setup`.  This will be persisted behind the scenes and passed to every callback (such as `draw` and `mouseDown`).  Each callback should return the new value of the state (or the old value if it doesn't change).

- There are no built-in variables like `width` and `mouseX`.  Instead, these are functions that are called, passing in an environment object that is always provided.
```reason ;prefix(1);no-run
open Reprocessing;
let draw = (state, env) => {
  let w = Env.width(env);
  print_endline("The current width is:" ++ string_of_int(w))
};
```

- The builtin `map` function is called `remap` instead to avoid confusion with the well-known `List.map` function which maps over a list of values. As, according to the Processing docs, this function "Re-maps a number from one range to another.", this naming seems appropriate.

- Points are expressed as tuples.  Instead of exposing a `mouseX` and `mouseY`, there is a `mouse`, which is a tuple of x and y values.

```reason ;prefix(1);no-run
open Reprocessing;
let draw = (state, env) => {
  let (x, y) = Env.mouse(env);
  print_endline("The current mouse position is:" ++ (string_of_int(x) ++ string_of_int(y)))
};
```

## Projects using Reprocessing

- [Gravitron](https://github.com/jaredly/gravitron)
- [Oh No! Zombies!](https://github.com/bsansouci/ludum-dare-40)
- [2048](https://github.com/bsansouci/reprocessing-example/tree/2048)
- [FlappyBird](https://github.com/bsansouci/reprocessing-example/tree/livestream-flappybird)
- [Chrome T-Rex](https://github.com/fabe/t-rex-runner-reason)
- [Pong](https://github.com/illbexyz/repong)
- [Ticky-Tacky](https://github.com/blaketarter/reticky-tacky)
- [Perlin Noise Flow Field](https://github.com/ekosz/reprocessing-example-flow-field)
- [L-System](https://github.com/Rigellute/L-system-reasonml)
- [My Dear Farm](https://github.com/bsansouci/ludum-dare-41)
- [Retro](https://github.com/jslauthor/reprocessing-retro)
- [Game Of Life](https://github.com/romanschejbal/reasonml-gol)
- [Tetris](https://github.com/rdavison/retetris)
- [Purple Maze](https://github.com/jaredly/purple-maze)
- [Egg](https://github.com/danieljharvey/reason-egg)
- [Remandelbrot](https://github.com/sscaff1/remandelbrot)

## Talks and articles about Reprocessing
- [Wonky Game Physics in Reason and Lessons Learned](https://www.youtube.com/watch?v=PUBJwiECPoc)
- [Making a cross-platform mobile game in Reason/OCaml](https://jaredforsyth.com/posts/making-a-cross-platform-mobile-game-in-reason-ocaml/)
- [2048 live-coding](https://www.youtube.com/watch?v=UDOEd5jS0Ac)
- [FlappyBird live-coding](https://www.youtube.com/watch?v=5aD3aPvNpyQ)

Please open a PR to add any cool projects, talks or articles about Reprocessing that are missing above!
