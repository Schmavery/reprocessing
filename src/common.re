open Glloader;

module Constants = Reglinterface.Constants;

type glState = Gl.Window.t;

type glCamera = {projectionMatrix: Gl.Mat4.t};

type color = {r: int, b: int, g: int};

type strokeT = {color: color, weight: int};

type mouseT = {pos: (int, int), prevPos: (int, int), pressed: bool};

type frameT = {count: int, rate: int};

type sizeT = {height: int, width: int, resizeable: bool};

type imageT = ref (option Gl.imageT);

type glEnv = {
  camera: glCamera,
  window: Gl.Window.t,
  gl: Gl.contextT,
  vertexBuffer: Gl.bufferT,
  aVertexColor: Gl.attributeT,
  aTextureCoord: Gl.attributeT,
  aVertexPosition: Gl.attributeT,
  pMatrixUniform: Gl.uniformT,
  uSampler: Gl.uniformT,
  uTextureFlag: Gl.uniformT,
  texture: Gl.textureT,
  currFill: color,
  currBackground: color,
  mouse: mouseT,
  stroke: strokeT,
  frame: frameT,
  size: sizeT
};

module type ReProcessorT = {
  type t;
  let run:
    setup::(ref glEnv => 'a) =>
    draw::('a => ref glEnv => 'a)? =>
    mouseMove::('a => ref glEnv => 'a)? =>
    mouseDragged::('a => ref glEnv => 'a)? =>
    mouseDown::('a => ref glEnv => 'a)? =>
    mouseUp::('a => ref glEnv => 'a)? =>
    unit =>
    unit;
};
