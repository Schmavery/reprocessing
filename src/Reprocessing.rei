/** Contains functions having to do with drawing to the screen */
module Draw = Reprocessing_Draw;

/** Contains functions having to do with the environment:
  * ie window properties, user input
*/
module Env = Reprocessing_Env;

/** Contains a number of type definitions used throughout
  * the library.
*/
module Common = Reprocessing_Common;

/** Contains types for events. */
module Events = Reasongl.Gl.Events;

/** Contains utility functions */
module Utils = Reprocessing_Utils;

/** Contains useful constants */
module Constants = Reprocessing_Constants;

/** Entrypoint to the graphics library. The system
  * is designed for you to return a self-defined 'state'
  * object from setup, which will then be passed to every
  * callback you choose to implement. Updating the state
  * is done by returning a different value from the callback.
*/
let run:
  setup::(Common.glEnv => 'a) =>
  draw::('a => Common.glEnv => 'a)? =>
  mouseMove::('a => Common.glEnv => 'a)? =>
  mouseDragged::('a => Common.glEnv => 'a)? =>
  mouseDown::('a => Common.glEnv => 'a)? =>
  mouseUp::('a => Common.glEnv => 'a)? =>
  keyPressed::('a => Common.glEnv => 'a)? =>
  keyReleased::('a => Common.glEnv => 'a)? =>
  keyTyped::('a => Common.glEnv => 'a)? =>
  unit =>
  unit;
