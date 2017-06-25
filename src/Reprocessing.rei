module Draw = Reprocessing_Draw;

module Env = Reprocessing_Env;

module Common = Reprocessing_Common;

module Events = Reasongl.Gl.Events;

module Utils = Reprocessing_Utils.PUtils;

module Constants = Reprocessing_Utils.PConstants;

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
