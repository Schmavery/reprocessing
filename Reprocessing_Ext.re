module Reprocessing = {
  type glEnvT;
  type colorT = {
    r: int,
    g: int,
    b: int
  };
  type strokeCapT =
    | Round
    | Square
    | Project;
  type imageT;
  type fontT;

  /** Contains types for events. */
  module Events = {
    type buttonStateT =
      | LeftButton
      | MiddleButton
      | RightButton;
    type stateT =
      | MouseDown
      | MouseUp;
    type keycodeT =
      | Backspace
      | Tab
      | Enter
      | Escape
      | Space
      | Quote
      | Comma
      | Minus
      | Period
      | Slash
      | Num_0
      | Num_1
      | Num_2
      | Num_3
      | Num_4
      | Num_5
      | Num_6
      | Num_7
      | Num_8
      | Num_9
      | Semicolon
      | Equals
      | OpenBracket
      | Backslash
      | CloseBracket
      | A
      | B
      | C
      | D
      | E
      | F
      | G
      | H
      | I
      | J
      | K
      | L
      | M
      | N
      | O
      | P
      | Q
      | R
      | S
      | T
      | U
      | V
      | W
      | X
      | Y
      | Z
      | Right
      | Left
      | Down
      | Up
      | LeftCtrl
      | LeftShift
      | LeftAlt
      | LeftOsKey
      | RightCtrl
      | RightShift
      | RightAlt
      | RightOsKey
      | CapsLock
      | Backtick
      | Nothing;
    external keycodeMap : int => keycodeT =
      "window.reprocessing.Events.keycodeMap" [@@bs.val];
  };
  module Draw = {
    external background : colorT => glEnvT => unit =
       "window.reprocessing.Draw.background" [@@bs.val];
    external translate : x::float => y::float => glEnvT => unit =
       "window.reprocessing.Draw.translate" [@@bs.val];
    external rotate : float => glEnvT => unit =
       "window.reprocessing.Draw.rotate" [@@bs.val];
    external fill : colorT => glEnvT => unit =
       "window.reprocessing.Draw.fill" [@@bs.val];
    external noFill : glEnvT => unit =
       "window.reprocessing.Draw.noFill" [@@bs.val];
    external stroke : colorT => glEnvT => unit =
       "window.reprocessing.Draw.stroke" [@@bs.val];
    external noStroke : glEnvT => unit =
       "window.reprocessing.Draw.noStroke" [@@bs.val];
    external strokeWeight : int => glEnvT => unit =
       "window.reprocessing.Draw.strokeWeight" [@@bs.val];
    external strokeCap : strokeCapT => glEnvT => unit =
       "window.reprocessing.Draw.strokeCap" [@@bs.val];
    external pushStyle : glEnvT => unit =
       "window.reprocessing.Draw.pushStyle" [@@bs.val];
    external popStyle : glEnvT => unit =
       "window.reprocessing.Draw.popStyle" [@@bs.val];
    external loadImage : filename::string => glEnvT => imageT =
       "window.reprocessing.Draw.loadImage" [@@bs.val];
    external _image :
      imageT =>
      pos::(int, int) =>
      width::option int =>
      height::option int =>
      glEnvT =>
      unit =
      "window.reprocessing.Draw.image" [@@bs.val];
    let image img ::pos ::width=? ::height=? env =>
      _image img ::pos ::width ::height env;
    external subImage :
      imageT =>
      pos::(int, int) =>
      width::int =>
      height::int =>
      texPos::(int, int) =>
      texWidth::int =>
      texHeight::int =>
      glEnvT =>
      unit =
       "window.reprocessing.Draw.subImage" [@@bs.val];
    external rectf :
      pos::(float, float) => width::float => height::float => glEnvT => unit =
       "window.reprocessing.Draw.rectf" [@@bs.val];
    external rect :
      pos::(int, int) => width::int => height::int => glEnvT => unit =
       "window.reprocessing.Draw.rect" [@@bs.val];
    external linef : p1::(float, float) => p2::(float, float) => glEnvT => unit =
       "window.reprocessing.Draw.linef" [@@bs.val];
    external line : p1::(int, int) => p2::(int, int) => glEnvT => unit =
       "window.reprocessing.Draw.line" [@@bs.val];
    external ellipsef :
      center::(float, float) => radx::float => rady::float => glEnvT => unit =
       "window.reprocessing.Draw.ellipsef" [@@bs.val];
    external ellipse :
      center::(int, int) => radx::int => rady::int => glEnvT => unit =
       "window.reprocessing.Draw.ellipse" [@@bs.val];
    external quadf :
      p1::(float, float) =>
      p2::(float, float) =>
      p3::(float, float) =>
      p4::(float, float) =>
      glEnvT =>
      unit =
       "window.reprocessing.Draw.quadf" [@@bs.val];
    external quad :
      p1::(int, int) =>
      p2::(int, int) =>
      p3::(int, int) =>
      p4::(int, int) =>
      glEnvT =>
      unit =
       "window.reprocessing.Draw.quad" [@@bs.val];
    external pixelf : pos::(float, float) => color::colorT => glEnvT => unit =
       "window.reprocessing.Draw.pixelf" [@@bs.val];
    external pixel : pos::(int, int) => color::colorT => glEnvT => unit =
       "window.reprocessing.Draw.pixel" [@@bs.val];
    external trianglef :
      p1::(float, float) =>
      p2::(float, float) =>
      p3::(float, float) =>
      glEnvT =>
      unit =
       "window.reprocessing.Draw.trianglef" [@@bs.val];
    external triangle :
      p1::(int, int) => p2::(int, int) => p3::(int, int) => glEnvT => unit =
       "window.reprocessing.Draw.triangle" [@@bs.val];
    external bezier :
      p1::(float, float) =>
      p2::(float, float) =>
      p3::(float, float) =>
      p4::(float, float) =>
      glEnvT =>
      unit =
       "window.reprocessing.Draw.bezier" [@@bs.val];
    external arcf :
      center::(float, float) =>
      radx::float =>
      rady::float =>
      start::float =>
      stop::float =>
      isOpen::bool =>
      isPie::bool =>
      glEnvT =>
      unit =
       "window.reprocessing.Draw.arcf" [@@bs.val];
    external arc :
      center::(int, int) =>
      radx::int =>
      rady::int =>
      start::float =>
      stop::float =>
      isOpen::bool =>
      isPie::bool =>
      glEnvT =>
      unit =
       "window.reprocessing.Draw.arc" [@@bs.val];
    external loadFont : filename::string => glEnvT => fontT =
       "window.reprocessing.Draw.loadFont" [@@bs.val];
    external text :
      font::fontT => body::string => pos::(int, int) => glEnvT => unit =
       "window.reprocessing.Draw.text" [@@bs.val];
    external clear : glEnvT => unit =
       "window.reprocessing.Draw.clear" [@@bs.val];
  };

  /** Contains functions having to do with the environment:
     * ie window properties, user input
   */
  module Env = {
    external width : glEnvT => int =
       "window.reprocessing.Env.width" [@@bs.val];
    external height : glEnvT => int =
       "window.reprocessing.Env.height" [@@bs.val];
    external mouse : glEnvT => (int, int) =
       "window.reprocessing.Env.mouse" [@@bs.val];
    external pmouse : glEnvT => (int, int) =
       "window.reprocessing.Env.pmouse" [@@bs.val];
    external mousePressed : glEnvT => bool =
       "window.reprocessing.Env.mousePressed" [@@bs.val];
    external keyCode : glEnvT => Events.keycodeT =
       "window.reprocessing.Env.keyCode" [@@bs.val];
    external size : width::int => height::int => glEnvT => unit =
       "window.reprocessing.Env.size" [@@bs.val];
    external resizeable : bool => glEnvT => unit =
       "window.reprocessing.Env.resizeable" [@@bs.val];
    external frameRate : glEnvT => int =
       "window.reprocessing.Env.frameRate" [@@bs.val];
    external frameCount : glEnvT => int =
       "window.reprocessing.Env.frameCount" [@@bs.val];
  };

  /** Contains utility functions */
  module Utils = {
    external color : r::int => g::int => b::int => colorT =
       "window.reprocessing.Utils.color" [@@bs.val];
    external round : float => float =
       "window.reprocessing.Utils.round" [@@bs.val];
    external sq : int => int =
       "window.reprocessing.Utils.sq" [@@bs.val];
    external pow : base::int => exp::int => int =
       "window.reprocessing.Utils.pow" [@@bs.val];
    external constrain : amt::'a => low::'a => high::'a => 'a =
       "window.reprocessing.Utils.constrain" [@@bs.val];
    external remapf :
      value::float =>
      low1::float =>
      high1::float =>
      low2::float =>
      high2::float =>
      float =
       "window.reprocessing.Utils.remapf" [@@bs.val];
    external remap :
      value::int => low1::int => high1::int => low2::int => high2::int => int =
       "window.reprocessing.Utils.remap" [@@bs.val];
    external norm : value::float => low::float => high::float => float =
       "window.reprocessing.Utils.norm" [@@bs.val];
    external randomf : min::float => max::float => float =
       "window.reprocessing.Utils.randomf" [@@bs.val];
    external random : min::int => max::int => int =
       "window.reprocessing.Utils.random" [@@bs.val];
    external randomSeed : int => unit =
       "window.reprocessing.Utils.randomSeed" [@@bs.val];
    external randomGaussian : unit => float =
       "window.reprocessing.Utils.randomGaussian" [@@bs.val];
    external lerpf : low::float => high::float => value::float => float =
       "window.reprocessing.Utils.lerpf" [@@bs.val];
    external lerp : low::int => high::int => value::float => int =
       "window.reprocessing.Utils.lerp" [@@bs.val];
    external lerpColor : low::colorT => high::colorT => value::float => colorT =
       "window.reprocessing.Utils.lerpColor" [@@bs.val];
    external distf : p1::(float, float) => p2::(float, float) => float =
       "window.reprocessing.Utils.distf" [@@bs.val];
    external dist : p1::(int, int) => p2::(int, int) => float =
       "window.reprocessing.Utils.dist" [@@bs.val];
    external magf : (float, float) => float =
       "window.reprocessing.Utils.magf" [@@bs.val];
    external mag : (int, int) => float =
       "window.reprocessing.Utils.mag" [@@bs.val];
    external degrees : float => float =
       "window.reprocessing.Utils.degrees" [@@bs.val];
    external radians : float => float =
       "window.reprocessing.Utils.radians" [@@bs.val];
    external noise : float => float => float => float =
       "window.reprocessing.Utils.noise" [@@bs.val];
    external noiseSeed : int => unit =
       "window.reprocessing.Utils.noiseSeed" [@@bs.val];
    external split : string => sep::char => list string =
       "window.reprocessing.Utils.split" [@@bs.val];
  };

  /** Contains useful constants */
  module Constants = {
    external white : colorT =
       "window.reprocessing.Constants.white" [@@bs.val];
    external black : colorT =
       "window.reprocessing.Constants.black" [@@bs.val];
    external red : colorT =
       "window.reprocessing.Constants.red" [@@bs.val];
    external green : colorT =
       "window.reprocessing.Constants.green" [@@bs.val];
    external blue : colorT =
       "window.reprocessing.Constants.blue" [@@bs.val];
    external pi : float =
       "window.reprocessing.Constants.pi" [@@bs.val];
    external half_pi : float =
       "window.reprocessing.Constants.half_pi" [@@bs.val];
    external quarter_pi : float =
       "window.reprocessing.Constants.quarter_pi" [@@bs.val];
    external two_pi : float =
       "window.reprocessing.Constants.two_pi" [@@bs.val];
    external tau : float =
       "window.reprocessing.Constants.tau" [@@bs.val];
  };

  /** Entrypoint to the graphics library. The system
     * is designed for you to return a self-defined 'state'
     * object from setup, which will then be passed to every
     * callback you choose to implement. Updating the state
     * is done by returning a different value from the callback.
   */
  external _run :
    setup::(glEnvT => 'a) =>
    draw::option ('a => glEnvT => 'a) =>
    mouseMove::option ('a => glEnvT => 'a) =>
    mouseDragged::option ('a => glEnvT => 'a) =>
    mouseDown::option ('a => glEnvT => 'a) =>
    mouseUp::option ('a => glEnvT => 'a) =>
    keyPressed::option ('a => glEnvT => 'a) =>
    keyReleased::option ('a => glEnvT => 'a) =>
    keyTyped::option ('a => glEnvT => 'a) =>
    unit =>
    unit =
    "window.reprocessing.run" [@@bs.val];
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
      () =>
    _run
      ::setup
      ::draw
      ::mouseMove
      ::mouseDragged
      ::mouseDown
      ::mouseUp
      ::keyPressed
      ::keyReleased
      ::keyTyped
      ();
};
