/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
open Glloader;

module Constants = Reglinterface.Constants;

type glState = Gl.Window.t;

type glCamera = {projectionMatrix: Gl.Mat4.t};

type colorT = {r: int, b: int, g: int};

type strokeT = {color: colorT, weight: int};

type mouseT = {pos: (int, int), prevPos: (int, int), pressed: bool};

type frameT = {count: int, rate: int};

type sizeT = {height: int, width: int, resizeable: bool};

let circularBufferSize = 6 * 10000;

type _imageT = {textureBuffer: Gl.textureT, img: Gl.imageT, height: int, width: int};

type imageT = ref (option _imageT);

type batchT = {
  vertexArray: Gl.Bigarray.t float Gl.Bigarray.float32_elt,
  elementArray: Gl.Bigarray.t int Gl.Bigarray.int16_unsigned_elt,
  mutable vertexPtr: int,
  mutable elementPtr: int,
  mutable currTex: Gl.textureT,
  nullTex: Gl.textureT
};

type glEnv = {
  camera: glCamera,
  window: Gl.Window.t,
  gl: Gl.contextT,
  vertexBuffer: Gl.bufferT,
  elementBuffer: Gl.bufferT,
  aVertexColor: Gl.attributeT,
  aTextureCoord: Gl.attributeT,
  aVertexPosition: Gl.attributeT,
  pMatrixUniform: Gl.uniformT,
  uSampler: Gl.uniformT,
  uTextureFlag: Gl.uniformT,
  batch: batchT,
  currFill: colorT,
  currBackground: colorT,
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

module Stream = {
  type t = (string, int);
  let empty = [];
  let peekch ((str, i): t) :option char =>
    if (i < String.length str) {
      Some str.[i]
    } else {
      None
    };
  let popch ((str, i): t) :t => (str, i + 1);
  let peekn (str, i) len =>
    if (i + len < String.length str) {
      Some (String.sub str i len)
    } else {
      None
    };
  let popn (str, i) len => (str, i + len);
  let match stream matchstr => {
    let len = String.length matchstr;
    switch (peekn stream len) {
    | Some peek when peek == matchstr => popn stream len
    | Some peek => failwith ("Could not match '" ^ matchstr ^ "', got '" ^ peek ^ "' instead.")
    | None => failwith ("Could not match " ^ matchstr)
    }
  };
  let charsRemaining (str, i) => String.length str - i;
  let create (str: string) :t => (str, 0);
};

let read (name: string) => {
  let ic = open_in name;
  let try_read () =>
    try (Some (input_line ic)) {
    | End_of_file => None
    };
  let rec loop acc =>
    switch (try_read ()) {
    | Some s => loop [String.make 1 '\n', s, ...acc]
    | None =>
      close_in ic;
      List.rev acc
    };
  loop [] |> String.concat ""
};

let append_char (s: string) (c: char) :string => s ^ String.make 1 c;
