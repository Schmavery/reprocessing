# ReProcessing

This is a high-level drawing library, heavily inspired by [Processing](https://processing.org) and built on top of [bsansouci/reasongl](https://github.com/bsansouci/reasongl).  This means you can write graphics code once, and have the (exact) same code compile to run on web (webgl) and native (opengl).
Everything you need should be accessible from the Reprocessing module.

### Example:
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
This will draw a simple red square on a black background.  Compare this to [reglexampleproject](https://github.com/bsansouci/reglexampleproject/blob/master/src/index.re), which takes 200+ lines to do the exact same thing.  This difference is even more notable on bigger projects.  Check out the code for a [draggable red square](https://github.com/Schmavery/reprocessing/blob/master/src/redsquare.re).

# Some Differences from Processing
- There is no magic - everything is proper Reason code.  This means that you have to call `ReProcessor.run` with the functions that you want to use.  You also have a couple of options about which utility modules to open.  It is recommended to `open Reprocessing` at the top, and then you can optionally open `P` and `PUtils` to gain more functionality and make it look more like Processing code.  An example of this can be seen above.

- You have a couple of options for state management, but we encourage the use of the `state` value that ReProcessing will manage for the user.  To use this, decide on a datatype representing the state and return the initial value from `setup`.  This will be persisted behind the scenes and passed to every callback (such as `draw` and `mouseDown`).  Each callback should return the new value of the state (or the old value if it doesn't change).  This will allow you to write event-driven code with no knowledge of reference types.

- There are no built-in variables like `width` and `mouseX`.  Instead, these are functions that are called on an environment object that is always provided.
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
