# Reprocessing

This is a high-level drawing library, heavily inspired by [Processing](https://processing.org) and built on top of [bsansouci/reasongl](https://github.com/bsansouci/reasongl).  This means you can write graphics code once, and have the (exact) same code compile to run on web (webgl) and native (opengl).
Everything you need should be accessible from the Reprocessing module.

If you have trouble installing or want to give any input for how to make this library better, please open an issue! :smile:

## Demo
There are a couple demos inside `examples`. Run `npm i` to install all deps and `npm run build` to build to JS (default). Open `index.html` in safari (or use `python -m SimpleHTTPServer 8000` to spawn a static server and go to `localhost:8000` in chrome).

Run `npm run build:bytecode` to build to a bytecode executable and run `./lib/bs/bytecode/index.byte`.

Run `npm run build:native` to build to a native executable and run `./lib/bs/native/index.native`.

## Install
```bash
npm install Schmavery/reprocessing#bsb-support-new
```

This is a library, meant to be installed as a dependency (though you can also clone and play in the `examples` directory).  It builds using [bsb-native](https://github.com/bsansouci/bsb-native), and you will need to install this as a devDependency of your project if you want to try out the native version. If you only want webgl support, regular [bs-platform](https://github.com/BuckleScript/bucklescript) should be fine.

## Example:
```reason
open Reprocessing;

let setup env => {
  Env.size width::600 height::600 env;
  Draw.fill Constants.red env
};

let draw state env => {
  Draw.background Constants.black env;
  Draw.rect pos::(150, 150) width::300 height::300 env
};

run ::setup ::draw ();
```
This will draw a simple red square on a black background.  Compare this to [reglexampleproject](https://github.com/bsansouci/reglexampleproject/blob/master/src/index.re), which takes 200+ lines to do the exact same thing.  This difference is even more notable on bigger projects.  Check out the code for a [draggable red square](https://github.com/Schmavery/reprocessing/blob/bsb-support-new/examples/redsquare.re).

# Some Differences from Processing
- There is no magic - everything is proper Reason code.  This means that you have to call `Reprocessing.run` with the functions that you want to use.  You also have a couple of options about which utility modules to open.  See the `examples` directory for some different ways to do this.  It is recommended to `open Reprocessing` at the top, and then you can optionally open `Draw`, `Env` and `Utils` to make it look more like Processing code. Alternatively, they can be used directly, as can be seen above.

- For state management, we encourage the use of the `state` value that Reprocessing manages for the user.  To use this, decide on a datatype representing the state and return the initial value from `setup`.  This will be persisted behind the scenes and passed to every callback (such as `draw` and `mouseDown`).  Each callback should return the new value of the state (or the old value if it doesn't change).

- There are no built-in variables like `width` and `mouseX`.  Instead, these are functions that are called, passing in an environment object that is always provided.
```reason
let draw state env => {
  let w = Env.width env;
  print_endline ("The current width is:" ^ string_of_int w)
};
```

- The builtin `map` function is called `remap` instead to avoid confusion with the well-known `List.map` function which maps over a list of values. As, according to the Processing docs, this function "Re-maps a number from one range to another.", this naming seems appropriate.

- Because of the limitations of Reason, several utility functions that would otherwise accept either an integer or a float now expose a version with an `f` suffix, which supports floats.  Ex: `random` vs `randomf`.

- Points are expressed as tuples.  Instead of exposing a `mouseX` and `mouseY`, there is a `mouse`, which is a tuple of x and y values.
```reason
let draw state env => {
  let (x, y) = Env.mouse env;
  print_endline ("The current mouse position is:" ^ string_of_int x ^ string_of_int y)
};
```

- This is 100% a work in progress.  Only a small subset of Processing's functionality has been replicated.  Please feel free to contribute as my knowledge of OpenGl is limited at best :)


# Using Fonts
The story for using fonts in your Reprocessing app is still under some development to make it nicer.  Right now we have support for writing text in a font defined in the [Angel Code font](http://www.angelcode.com/products/bmfont/) format. This is basically a bitmap of packed glyph textures along with a text file that describes it. The above link gives a few different tools that can be used to generate these files.  The assets folder of this repo also has an [example](https://github.com/Schmavery/reprocessing/tree/bsb-support-new/assets/font) of a font that can be copied to your project and used.  In order to use a font once you have the files:
```
let font = Draw.loadFont ::filename env;
Draw.text ::font body::"Test!!!" pos::(10, 10) env
```
