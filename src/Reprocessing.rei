/*** Contains functions having to do with drawing to the screen */
module Draw = Reprocessing_Draw;


/*** Contains functions having to do with the environment:
   * ie window properties, user input
 */
module Env = Reprocessing_Env;


/*** Contains types for events. */
module Events = Reprocessing_Events;


/*** Contains utility functions */
module Utils = Reprocessing_Utils;


/*** Contains useful constants */
module Constants = Reprocessing_Constants;

include Reprocessing_Types.TypesT;

let hotreload: string => bool;


/*** Entrypoint to the graphics library. The system
   * is designed for you to return a self-defined 'state'
   * object from setup, which will then be passed to every
   * callback you choose to implement. Updating the state
   * is done by returning a different value from the callback.
 */
let run:
  (
    ~setup: glEnvT => 'a,
    ~draw: ('a, glEnvT) => 'a=?,
    ~mouseMove: ('a, glEnvT) => 'a=?,
    ~mouseDragged: ('a, glEnvT) => 'a=?,
    ~mouseDown: ('a, glEnvT) => 'a=?,
    ~mouseUp: ('a, glEnvT) => 'a=?,
    ~keyPressed: ('a, glEnvT) => 'a=?,
    ~keyReleased: ('a, glEnvT) => 'a=?,
    ~keyTyped: ('a, glEnvT) => 'a=?,
    unit
  ) =>
  unit;
