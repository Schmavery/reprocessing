'use strict';

var codeMirrorDefaultHeight = 10000;
var myCode1Mirror = CodeMirror.fromTextArea(
  document.getElementById('ocamlcode#1'),
  {
    mode:'text/x-ocaml',
    lineNumbers: true,
    lineWrapping: true,
    styleActiveLine:true,
    theme: "monokai",
    matchBrackets: true,
    autoCloseBrackets: true
  } );
var jsCode1Mirror = CodeMirror.fromTextArea(
  document.getElementById('jscode#1'),
  { mode:'javascript',
    lineNumbers:true,
    readOnly: true,
    lineWrapping: true,
    theme: "monokai",
  });

var outputMirror = CodeMirror.fromTextArea(
  document.getElementById('output'),
  {
    mode : 'javascript',
    readOnly: true,
    lineWrapping: true,
    lineNumbers: true,
    theme: "monokai"
  }
);
var errorMirror = CodeMirror.fromTextArea(
  document.getElementById('error'),
  {
    readOnly: true,
    lineWrapping: true,
    lineNumbers : true,
    theme: "monokai"
  }
);
var PROMPT = "> " ;
var log_output = PROMPT;
var ERR_OUTPUT = "Warnings: "
var err_output = ERR_OUTPUT;

function reset_log_output (){ log_output  = PROMPT;}
function reset_error_output(){ err_output = ERR_OUTPUT;}
function get_log_output(){
  var old = log_output;
  reset_log_output();
  return old;
}
function get_error_output(){
  var old = err_output;
  reset_error_output();
  return old;
}
var compile_code ;
var evalButton = document.getElementById('option-eval');
var shakeButton = document.getElementById('option-non-export');

function shouldEval(){
  return evalButton.checked;
}
function onEvalButtonChange(){
  if(!shouldEval()){
    outputMirror.setValue(PROMPT);
  } else {
    onEditChanges();
  }
}
evalButton.addEventListener('change', onEvalButtonChange);

function onShakeButtonChange(){
  if(shakeButton.checked){
    compile_code = ocaml.shake_compile;
  }else{
    compile_code = ocaml.compile;
  }
  onEditChanges();
}

shakeButton.addEventListener('change', onShakeButtonChange);
var original_log = console.log;
var original_err = console.error;

/**
 * TODO: simulate commonjs in browser
 * */
var exports = window;

function redirect() { log_output = log_output + Array.prototype.slice.apply(arguments).join(' ') + "\n"};

function redirect_err() {
    err_output = err_output + Array.prototype.slice.apply(arguments).join(' ') + "\n"
};

myCode1Mirror.setSize(null,codeMirrorDefaultHeight);
outputMirror.setSize(null,50);
outputMirror.setValue(PROMPT + 'Hello BuckleScript!');
errorMirror.setSize(null,50);
errorMirror.setValue(ERR_OUTPUT);

var sourceLocation = ""
if (typeof window.location !== "undefined"){
    sourceLocation = "\n//# sourceURL=" + window.location.href + "/repl.js"
}

function evalCode(js){
  console.log = redirect;
  try {
    window.eval(js + sourceLocation);
    outputMirror.setValue(get_log_output());
    console.log = original_log;
  }
  catch(e){
    outputMirror.setValue(get_log_output() + "\n" + e);
    console.log = original_log;
  }

}

function createExample(name){
    var li = document.createElement('li');
    var a = document.createElement('a')
    a.setAttribute('href', '#' + name);
    a.setAttribute('data-key', name);
    a.appendChild(document.createTextNode(name))
    li.appendChild(a)
    return li
}

var examplesDropdown = document.getElementById("examplesDropdown")
var examplesDataSet ;


//Event handler for examples dropdown
$('#examplesDropdown').click(clickHandler);

function switchExample(id){
    var filename = "";
    var example = examplesDataSet [id];
    if (example){
        changeEvalButton(example.eval)
        filename = "examples/" + example.file
    }
    //make ajax request
    $
    .ajax({
        url: filename,
        cache: true
    })
    .done(function (response) {
        myCode1Mirror.setValue(response);
    });

    //update dropdown label
    $('#examplesLabel').html(id + ' <span class="caret"></span>');

}

function clickHandler(e) {
    var id = e.target.getAttribute('data-key');
    switchExample(id)
}

var predefinedStuff = `
module Reglinterface =
struct
#1 "reglinterface.ml"
module Constants = struct
let triangles = 4
let triangle_strip = 5
let texture0 = 33984
let texture1 = 33985
let texture2 = 33986
let texture_2d = 3553
let blend = 3042
let texture_wrap_s = 10242
let texture_wrap_t = 10243
let clamp_to_edge = 33071
let src_alpha = 770
let one_minus_src_alpha = 771
let dst_alpha = 772
let depth_test = 2929
let rgb = 6407
let rgba = 6408
let triangle_fan = 6
let array_buffer = 34962
let element_array_buffer = 34963
let array_buffer_binding = 34964
let element_array_buffer_binding = 34965
let stream_draw = 35040
let static_draw = 35044
let dynamic_draw = 35048
let buffer_size = 34660
let buffer_usage = 34661
let float_ = 5126
let unsigned_int = 5125
let current_vertex_attrib = 34342
let fragment_shader = 35632
let vertex_shader = 35633
let max_vertex_attribs = 34921
let max_vertex_uniform_vectors = 36347
let max_varying_vectors = 36348
let max_combined_texture_image_units = 35661
let max_vertex_texture_image_units = 35660
let max_texture_image_units = 34930
let max_fragment_uniform_vectors = 36349
let shader_type = 35663
let delete_status = 35712
let link_status = 35714
let validate_status = 35715
let attached_shaders = 35717
let active_uniforms = 35718
let active_attributes = 35721
let shading_language_version = 35724
let current_program = 35725
let compile_status = 35713
let vendor = 7936
let renderer = 7937
let version = 7938
let float_vec2 = 35664
let float_vec3 = 35665
let float_vec4 = 35666
let int_vec2 = 35667
let int_vec3 = 35668
let int_vec4 = 35669
let bool_ = 35670
let bool_vec2 = 35671
let bool_vec3 = 35672
let bool_vec4 = 35673
let float_mat2 = 35674
let float_mat3 = 35675
let float_mat4 = 35676
let sampler_2d = 35678
let sampler_cube = 35680
let unpack_flip_y_webgl = 37440
let unpack_premultiply_alpha_webgl = 37441
let context_lost_webgl = 37442
let unpack_colorspace_conversion_webgl = 37443
let browser_default_webgl = 37444
let vertex_attrib_array_enabled = 34338
let vertex_attrib_array_size = 34339
let vertex_attrib_array_stride = 34340
let vertex_attrib_array_type = 34341
let vertex_attrib_array_normalized = 34922
let vertex_attrib_array_pointer = 34373
let vertex_attrib_array_buffer_binding = 34975
let depth_buffer_bit = 256
let stencil_buffer_bit = 1024
let color_buffer_bit = 16384
let unsigned_short = 5123
let unsigned_byte = 5121
let texture_mag_filter = 10240
let texture_min_filter = 10241
let linear = 9729
let linear_mipmap_nearest = 9985
end
end
module Glloader
= struct
#1 "glloader.ml"
module Document =
  struct
    type element
    type window
    let window: window = [%bs.raw "window"]
    external getElementById : string -> element = "document.getElementById"
    [@@bs.val ]
    external getContext : element -> string -> 'context = "getContext"
    [@@bs.send ]
    external getWidth : element -> int = "width"[@@bs.get ]
    external getHeight : element -> int = "height"[@@bs.get ]
    external requestAnimationFrame :
      (unit -> unit) -> unit = "window.requestAnimationFrame"[@@bs.val ]
    external now : unit -> float = "Date.now"[@@bs.val ]
    external addEventListener :
      'window -> string -> ('eventT -> unit) -> unit = "addEventListener"
    [@@bs.send ]
  end
external getButton : 'eventT -> int = "button"[@@bs.get ]
external getClientX : 'eventT -> int = "clientX"[@@bs.get ]
external getClientY : 'eventT -> int = "clientY"[@@bs.get ]
external getWidth : 'canvas -> int = "width"[@@bs.get ]
external getHeight : 'canvas -> int = "height"[@@bs.get ]
external setWidth : 'canvas -> int -> unit = "width"[@@bs.set ]
external setHeight : 'canvas -> int -> unit = "height"[@@bs.set ]
external createElement : string -> 'canvas = "document.createElement"[@@bs.val
                                                                    ]
let createCanvas () = createElement "canvas"
external addToBody : 'canvas -> unit = "document.body.appendChild"[@@bs.val ]
external getContext :
  'canvas -> string -> 'options -> 'context = "getContext"[@@bs.send ]
type styleT
external getStyle : 'canvas -> styleT = "style"[@@bs.get ]
external setBackgroundColor : styleT -> string -> unit = "backgroundColor"
[@@bs.set ]
module Gl = struct
    let target = "web"
    type contextT
    module type WindowT  =
      sig
        type t
        val getWidth : t -> int
        val getHeight : t -> int
        val initWithCanvasID : string -> t
        val init : argv:string array -> t
        val setWindowSize : window:t -> width:int -> height:int -> unit
        val initDisplayMode : window:t -> double_buffer:bool -> unit -> unit
        val getContext : t -> contextT
      end
    module Window =
      struct
        type t
        external getElementByIdForCanvasGosh : string -> t = "document.getElementById" [@@bs.val]
        let getWidth = getWidth
        let getHeight = getHeight
        let initWithCanvasID canvasID = getElementByIdForCanvasGosh canvasID
        let init ~argv:_  =
          let canvas: t = createCanvas () in
          setBackgroundColor (getStyle canvas) "black";
          addToBody canvas;
          canvas
        let setWindowSize ~window:(window : t)  ~width  ~height  =
          setWidth window width; setHeight window height
        let initDisplayMode ~window  ~double_buffer:_  () = ()
        let getContext (window : t) =
          (getContext window "webgl"
             ([%bs.obj { preserveDrawingBuffer = true }]) : contextT)
      end
    module type EventsT  =
      sig
        type buttonStateT =
          | LEFT_BUTTON
          | MIDDLE_BUTTON
          | RIGHT_BUTTON
        type stateT =
          | DOWN
          | UP
      end
    module Events =
      struct
        type buttonStateT =
          | LEFT_BUTTON
          | MIDDLE_BUTTON
          | RIGHT_BUTTON
        type stateT =
          | DOWN
          | UP
      end
    type mouseButtonEventT =
      button:Events.buttonStateT ->
        state:Events.stateT -> x:int -> y:int -> unit
    let render ~window:(window : Window.t)
      ?mouseDown:(mouseDown : mouseButtonEventT option)
      ?mouseUp:(mouseUp : mouseButtonEventT option)
      ?mouseMove:(mouseMove : (x:int -> y:int -> unit) option)
      ?windowResize:(windowResize : (unit -> unit) option)
      ~displayFunc:(displayFunc : float -> unit)  () =
      (match mouseDown with
       | None  -> ()
       | ((Some (cb))[@explicit_arity ]) ->
           Document.addEventListener window "mousedown"
             (fun e  ->
                let button =
                  match getButton e with
                  | 0 -> Events.LEFT_BUTTON
                  | 1 -> Events.MIDDLE_BUTTON
                  | 2 -> Events.RIGHT_BUTTON
                  | _ -> assert false in
                let state = Events.DOWN in
                let x = getClientX e in
                let y = getClientY e in cb ~button ~state ~x ~y));
      (match mouseUp with
       | None  -> ()
       | ((Some (cb))[@explicit_arity ]) ->
           Document.addEventListener window "mouseup"
             (fun e  ->
                let button =
                  match getButton e with
                  | 0 -> Events.LEFT_BUTTON
                  | 1 -> Events.MIDDLE_BUTTON
                  | 2 -> Events.RIGHT_BUTTON
                  | _ -> assert false in
                let state = Events.UP in
                let x = getClientX e in
                let y = getClientY e in cb ~button ~state ~x ~y));
      (match mouseMove with
       | None  -> ()
       | ((Some (cb))[@explicit_arity ]) ->
           Document.addEventListener window "mousemove"
             (fun e  ->
                let x = getClientX e in let y = getClientY e in cb ~x ~y));
      (let rec tick prev () =
         let now = Document.now () in
         displayFunc (now -. prev); Document.requestAnimationFrame (tick now) in
       Document.requestAnimationFrame (tick (Document.now ())))
    type programT
    type shaderT
    external clearColor :
      context:contextT -> r:float -> g:float -> b:float -> a:float -> unit =
        "clearColor"[@@bs.send ]
    external createProgram : context:contextT -> programT = "createProgram"
    [@@bs.send ]
    external createShader :
      context:contextT -> shaderType:int -> shaderT = "createShader"[@@bs.send
                                                                    ]
    external _shaderSource :
      context:contextT -> shader:shaderT -> source:string -> unit =
        "shaderSource"[@@bs.send ]
    let shaderSource ~context  ~shader  ~source  =
      _shaderSource ~context ~shader
        ~source:("#version 100 \\n precision highp float; \\n" ^ source)
    external compileShader :
      context:contextT -> shader:shaderT -> unit = "compileShader"[@@bs.send
                                                                    ]
    external attachShader :
      context:contextT -> program:programT -> shader:shaderT -> unit =
        "attachShader"[@@bs.send ]
    external deleteShader :
      context:contextT -> shader:shaderT -> unit = "deleteShader"[@@bs.send ]
    external linkProgram :
      context:contextT -> program:programT -> unit = "linkProgram"[@@bs.send
                                                                    ]
    external useProgram :
      context:contextT -> program:programT -> unit = "useProgram"[@@bs.send ]
    type bufferT
    type attributeT
    type uniformT
    external createBuffer : context:contextT -> bufferT = "createBuffer"
    [@@bs.send ]
    external bindBuffer :
      context:contextT -> target:int -> buffer:bufferT -> unit = "bindBuffer"
    [@@bs.send ]
    type textureT
    external createTexture : context:contextT -> textureT = "createTexture"
    [@@bs.send ]
    external activeTexture :
      context:contextT -> target:int -> unit = "activeTexture"[@@bs.send ]
    external bindTexture :
      context:contextT -> target:int -> texture:textureT -> unit =
        "bindTexture"[@@bs.send ]
    external texParameteri :
      context:contextT -> target:int -> pname:int -> param:int -> unit =
        "texParameteri"[@@bs.send ]
    type uint8array
    type rawTextureDataT = uint8array
    external makeUint8ArrayWithLength : int -> rawTextureDataT = "Uint8Array"
    [@@bs.new ]
    external makeUint8Array : int array -> rawTextureDataT = "Uint8Array"
    [@@bs.new ]
    let toTextureData data = makeUint8Array data
    external enable : context:contextT -> int -> unit = "enable"[@@bs.send ]
    external disable : context:contextT -> int -> unit = "disable"[@@bs.send
                                                                    ]
    external blendFunc : context:contextT -> int -> int -> unit = "blendFunc"
    [@@bs.send ]
    type frameBufferT = int
    external createFrameBuffer :
      context:contextT -> frameBufferT = "createFramebuffer"[@@bs.send ]
    external _bindFrameBuffer :
      context:contextT -> target:int -> frameBuffer:'frameBufferT -> unit =
        "bindFramebuffer"[@@bs.send ]
    let bindFrameBuffer ~context  ~target  ~frameBuffer  =
      match frameBuffer with
      | None  -> _bindFrameBuffer context target Js.null
      | ((Some (frameBuffer))[@explicit_arity ]) ->
          _bindFrameBuffer context target frameBuffer
    external framebufferTexture2d :
      context:contextT ->
        target:int ->
          attachment:int ->
            texTarget:int -> texture:textureT -> level:int -> unit =
        "framebufferTexture2D"[@@bs.send ]
    external readPixels :
      context:contextT ->
        x:int ->
          y:int ->
            width:int ->
              height:int ->
                format:int -> type_:int -> pixels:rawTextureDataT -> unit =
        "readPixels"[@@bs.send ]
    let readPixelsRGBA ~context  ~x  ~y  ~width  ~height  =
      let data = makeUint8ArrayWithLength ((width * height) * 4) in
      readPixels ~context ~x ~y ~width ~height
        ~format:Reglinterface.Constants.rgba
        ~type_:Reglinterface.Constants.unsigned_byte ~pixels:data;
      data
    type imageT
    external getImageWidth : imageT -> int = "width"[@@bs.get ]
    external getImageHeight : imageT -> int = "height"[@@bs.get ]
    type loadOptionT =
      | LoadAuto
      | LoadL
      | LoadLA
      | LoadRGB
      | LoadRGBA
    external makeImage : unit -> imageT = "Image"[@@bs.new ]
    external setSrc : imageT -> string -> unit = "src"[@@bs.set ]
    external addEventListener :
      imageT -> string -> (unit -> unit) -> unit = "addEventListener"
    [@@bs.send ]
    let loadImage ~filename  ?loadOption  ~callback  () =
      match loadOption with
      | _ ->
          let image = makeImage () in
          (setSrc image filename;
           addEventListener image "load" (fun ()  -> callback (Some image)))
    external _texImage2DWithImage :
      context:contextT ->
        target:int ->
          level:int ->
            internalFormat:int ->
              format:int -> type_:int -> image:imageT -> unit = "texImage2D"
    [@@bs.send ]
    let texImage2DWithImage ~context  ~target  ~level  ~image  =
      _texImage2DWithImage context target level Reglinterface.Constants.rgba
        Reglinterface.Constants.rgba Reglinterface.Constants.unsigned_byte
        image
    external _texImage2D :
      context:contextT ->
        target:int ->
          level:int ->
            internalFormat:int ->
              width:int ->
                height:int ->
                  border:int ->
                    format:int -> type_:int -> data:rawTextureDataT -> unit =
        "texImage2D"[@@bs.send ]
    let texImage2D ~context  ~target  ~level  ~internalFormat  ~width
      ~height  ~format  ~type_  ~data  =
      _texImage2D context target level internalFormat width height 0 format
        type_ data
    external generateMipmap :
      context:contextT -> target:int -> unit = "generateMipmap"[@@bs.send ]
    external createFloat32ArrayOfArray :
      float array -> 'flot32array = "Float32Array"[@@bs.new ]
    external createFloat32Array : int -> 'float32array = "Float32Array"
    [@@bs.new ]
    external createFloat64ArrayOfArray :
      float array -> 'flot64array = "Float64Array"[@@bs.new ]
    external createFloat64Array : int -> 'float64array = "Float64Array"
    [@@bs.new ]
    external createIntArrayOfArray : int array -> 'int32array = "Int32Array"
    [@@bs.new ]
    external createInt32ArrayOfArray :
      int32 array -> 'int32array = "Int32Array"[@@bs.new ]
    external createIntArray : int -> 'int32array = "Int32Array"[@@bs.new ]
    external createInt32Array : int -> 'int32array = "Int32Array"[@@bs.new ]
    external createUint16ArrayOfArray :
      int array -> 'uint16array = "Uint16Array"[@@bs.new ]
    external createUint16Array : int -> 'uint16array = "Uint16Array"[@@bs.new
                                                                    ]
    external createInt16ArrayOfArray :
      int array -> 'int16array = "Int16Array"[@@bs.new ]
    external createInt16Array : int -> 'int16array = "Int16Array"[@@bs.new ]
    external createUint8ArrayOfArray :
      int array -> 'uint8array = "Uint8Array"[@@bs.new ]
    external createUint8Array : int -> 'uint8array = "Uint8Array"[@@bs.new ]
    external createInt8ArrayOfArray : int array -> 'int8array = "Int8Array"
    [@@bs.new ]
    external createInt8Array : int -> 'int8array = "Int8Array"[@@bs.new ]
    external createCharArrayOfArray :
      char array -> 'uint8array = "Uint8Array"[@@bs.new ]
    external sub : 'a -> int -> int -> 'a = "subarray"[@@bs.send ]
    module type Bigarray  =
      sig
        type ('a,'b) t
        type float64_elt
        type float32_elt
        type int16_unsigned_elt
        type int16_signed_elt
        type int8_unsigned_elt
        type int8_signed_elt
        type int_elt
        type int32_elt
        type int64_elt
        type ('a,'b) kind =
          | Float64: (float,float64_elt) kind
          | Float32: (float,float32_elt) kind
          | Int16: (int,int16_signed_elt) kind
          | Uint16: (int,int16_unsigned_elt) kind
          | Int8: (int,int8_signed_elt) kind
          | Uint8: (int,int8_unsigned_elt) kind
          | Char: (char,int8_unsigned_elt) kind
          | Int: (int,int_elt) kind
          | Int64: (int64,int64_elt) kind
          | Int32: (int32,int32_elt) kind
        val create : ('a,'b) kind -> int -> ('a,'b) t
        val of_array : ('a,'b) kind -> 'a array -> ('a,'b) t
        val dim : ('a,'b) t -> int
        val get : ('a,'b) t -> int -> 'a
        val set : ('a,'b) t -> int -> 'a -> unit
        val sub : ('a,'b) t -> offset:int -> len:int -> ('a,'b) t
      end
    module Bigarray =
      struct
        type ('a,'b) t
        type float64_elt
        type float32_elt
        type int16_unsigned_elt
        type int16_signed_elt
        type int8_unsigned_elt
        type int8_signed_elt
        type int_elt
        type int32_elt
        type int64_elt
        type ('a,'b) kind =
          | Float64: (float,float64_elt) kind
          | Float32: (float,float32_elt) kind
          | Int16: (int,int16_signed_elt) kind
          | Uint16: (int,int16_unsigned_elt) kind
          | Int8: (int,int8_signed_elt) kind
          | Uint8: (int,int8_unsigned_elt) kind
          | Char: (char,int8_unsigned_elt) kind
          | Int: (int,int_elt) kind
          | Int64: (int64,int64_elt) kind
          | Int32: (int32,int32_elt) kind
        let create (type a) (type b) (kind : (a,b) kind) size =
          (match kind with
           | Float64  -> createFloat64Array size
           | Float32  -> createFloat32Array size
           | Int16  -> createInt16Array size
           | Uint16  -> createUint16Array size
           | Int8  -> createInt8Array size
           | Uint8  -> createUint8Array size
           | Char  -> createUint8Array size
           | Int  -> createIntArray size
           | Int32  -> createInt32Array size
           | Int64  -> assert false : (a,b) t)
        let of_array (type a) (type b) (kind : (a,b) kind) (arr : a array) =
          (match kind with
           | Float64  -> createFloat64ArrayOfArray arr
           | Float32  -> createFloat32ArrayOfArray arr
           | Int16  -> createInt16ArrayOfArray arr
           | Uint16  -> createUint16ArrayOfArray arr
           | Int8  -> createInt8ArrayOfArray arr
           | Uint8  -> createUint8ArrayOfArray arr
           | Char  -> createCharArrayOfArray arr
           | Int  -> createIntArrayOfArray arr
           | Int32  -> createInt32ArrayOfArray arr
           | Int64  -> assert false : (a,b) t)
        external dim : 'a -> int = "length"[@@bs.get ]
        external get : ('a,'b) t -> int -> 'a = ""[@@bs.get_index ]
        external set : ('a,'b) t -> int -> 'a -> unit = ""[@@bs.set_index ]
        let sub arr ~offset  ~len  = sub arr offset (offset + len)
      end
    external bufferData :
      context:contextT ->
        target:int -> data:('a,'b) Bigarray.t -> usage:int -> unit =
        "bufferData"[@@bs.send ]
    external viewport :
      context:contextT -> x:int -> y:int -> width:int -> height:int -> unit =
        "viewport"[@@bs.send ]
    external clear : context:contextT -> mask:int -> unit = "clear"[@@bs.send
                                                                    ]
    external getUniformLocation :
      context:contextT -> program:programT -> name:string -> uniformT =
        "getUniformLocation"[@@bs.send ]
    external getAttribLocation :
      context:contextT -> program:programT -> name:string -> attributeT =
        "getAttribLocation"[@@bs.send ]
    external enableVertexAttribArray :
      context:contextT -> attribute:attributeT -> unit =
        "enableVertexAttribArray"[@@bs.send ]
    external _vertexAttribPointer :
      context:contextT ->
        attribute:attributeT ->
          size:int ->
            type_:int ->
              normalize:Js.boolean -> stride:int -> offset:int -> unit =
        "vertexAttribPointer"[@@bs.send ]
    let vertexAttribPointer ~context  ~attribute  ~size  ~type_  ~normalize
      ~stride  ~offset  =
      let normalize = if normalize then Js.true_ else Js.false_ in
      _vertexAttribPointer ~context ~attribute ~size ~type_ ~normalize
        ~stride ~offset
    module type Mat4T  =
      sig
        type t
        val to_array : t -> float array
        val create : unit -> t
        val identity : out:t -> unit
        val translate : out:t -> matrix:t -> vec:float array -> unit
        val scale : out:t -> matrix:t -> vec:float array -> unit
        val rotate :
          out:t -> matrix:t -> rad:float -> vec:float array -> unit
        val ortho :
          out:t ->
            left:float ->
              right:float ->
                bottom:float -> top:float -> near:float -> far:float -> unit
      end
    module Mat4 : Mat4T =
      struct
        type t = float array
        let to_array a = a
        external create : unit -> t = "mat4.create"[@@bs.val ]
        external identity : out:t -> unit = "mat4.identity"[@@bs.val ]
        external translate :
          out:t -> matrix:t -> vec:float array -> unit = "mat4.translate"
        [@@bs.val ]
        external scale :
          out:t -> matrix:t -> vec:float array -> unit = "mat4.scale"
        [@@bs.val ]
        external rotate :
          out:t -> matrix:t -> rad:float -> vec:float array -> unit =
            "mat4.rotate"[@@bs.val ]
        external ortho :
          out:t ->
            left:float ->
              right:float ->
                bottom:float -> top:float -> near:float -> far:float -> unit
            = "mat4.ortho"[@@bs.val ]
      end
    external uniform1i :
      context:contextT -> location:uniformT -> int -> unit = "uniform1i"
    [@@bs.send ]
    external uniform1f :
      context:contextT -> location:uniformT -> float -> unit = "uniform1f"
    [@@bs.send ]
    external _uniformMatrix4fv :
      context:contextT ->
        location:uniformT -> transpose:Js.boolean -> value:Mat4.t -> unit =
        "uniformMatrix4fv"[@@bs.send ]
    let uniformMatrix4fv ~context  ~location  ~value  =
      _uniformMatrix4fv ~context ~location ~transpose:Js.false_ ~value
    type 'a shaderParamsInternalT =
      | Shader_delete_status_internal: bool shaderParamsInternalT
      | Compile_status_internal: bool shaderParamsInternalT
      | Shader_type_internal: int shaderParamsInternalT
    type 'a programParamsInternalT =
      | Program_delete_status_internal: bool programParamsInternalT
      | Link_status_internal: bool programParamsInternalT
      | Validate_status_internal: bool programParamsInternalT
    type shaderParamsT =
      | Shader_delete_status
      | Compile_status
      | Shader_type
    type programParamsT =
      | Program_delete_status
      | Link_status
      | Validate_status
    external deleteStatus : context:contextT -> int = "DELETE_STATUS"
    [@@bs.get ]
    external compileStatus : context:contextT -> int = "COMPILE_STATUS"
    [@@bs.get ]
    external linkStatus : context:contextT -> int = "LINK_STATUS"[@@bs.get ]
    external validateStatus : context:contextT -> int = "VALIDATE_STATUS"
    [@@bs.get ]
    external shaderType : context:contextT -> int = "SHADER_TYPE"[@@bs.get ]
    external _getProgramParameter :
      context:contextT ->
        program:programT ->
          paramName:int -> (('a programParamsInternalT)[@bs.ignore ]) -> 'a =
        "getProgramParameter"[@@bs.send ]
    let getProgramParameter ~context  ~program  ~paramName  =
      match paramName with
      | Program_delete_status  ->
          if
            _getProgramParameter ~context ~program
              ~paramName:(deleteStatus ~context)
              Program_delete_status_internal
          then 1
          else 0
      | Link_status  ->
          if
            _getProgramParameter ~context ~program
              ~paramName:(linkStatus ~context) Link_status_internal
          then 1
          else 0
      | Validate_status  ->
          if
            _getProgramParameter ~context ~program
              ~paramName:(validateStatus ~context) Validate_status_internal
          then 1
          else 0
    external _getShaderParameter :
      context:contextT ->
        shader:shaderT ->
          paramName:int -> (('a shaderParamsInternalT)[@bs.ignore ]) -> 'a =
        "getShaderParameter"[@@bs.send ]
    let getShaderParameter ~context  ~shader  ~paramName  =
      match paramName with
      | Shader_delete_status  ->
          if
            _getShaderParameter ~context ~shader
              ~paramName:(deleteStatus ~context)
              Shader_delete_status_internal
          then 1
          else 0
      | Compile_status  ->
          if
            _getShaderParameter ~context ~shader
              ~paramName:(compileStatus ~context) Compile_status_internal
          then 1
          else 0
      | Shader_type  ->
          _getShaderParameter ~context ~shader
            ~paramName:(shaderType ~context) Shader_type_internal
    external getShaderInfoLog :
      context:contextT -> shader:shaderT -> string = "getShaderInfoLog"
    [@@bs.send ]
    external getProgramInfoLog :
      context:contextT -> program:programT -> string = "getProgramInfoLog"
    [@@bs.send ]
    external getShaderSource :
      context:contextT -> shader:shaderT -> string = "getShaderSource"
    [@@bs.send ]
    external drawArrays :
      context:contextT -> mode:int -> first:int -> count:int -> unit =
        "drawArrays"[@@bs.send ]
    external drawElements :
      context:contextT ->
        mode:int -> count:int -> type_:int -> offset:int -> unit =
        "drawElements"[@@bs.send ]
  end
end
module Common
= struct
#1 "common.ml"
open Glloader
module Constants = Reglinterface.Constants
type glState = Gl.Window.t
type glCamera = {
  projectionMatrix: Gl.Mat4.t;}
type colorT = {
  r: int;
  b: int;
  g: int;}
type strokeT = {
  color: colorT;
  weight: int;}
type mouseT = {
  pos: (int* int);
  prevPos: (int* int);
  pressed: bool;}
type frameT = {
  count: int;
  rate: int;}
type sizeT = {
  height: int;
  width: int;
  resizeable: bool;}
type _imageT =
  {
  img: Gl.imageT;
  textureBuffer: Gl.textureT;
  height: int;
  width: int;}
type imageT = _imageT option ref
type glEnv =
  {
  camera: glCamera;
  window: Gl.Window.t;
  gl: Gl.contextT;
  vertexBuffer: Gl.bufferT;
  elementBuffer: Gl.bufferT;
  aVertexColor: Gl.attributeT;
  aTextureCoord: Gl.attributeT;
  aVertexPosition: Gl.attributeT;
  pMatrixUniform: Gl.uniformT;
  uSampler: Gl.uniformT;
  uTextureFlag: Gl.uniformT;
  texture: Gl.textureT;
  currFill: colorT;
  currBackground: colorT;
  mouse: mouseT;
  stroke: strokeT;
  frame: frameT;
  size: sizeT;}
module type ReProcessorT  =
  sig
    type t
    val run :
      setup:(glEnv ref -> 'a) ->
        ?draw:('a -> glEnv ref -> 'a) ->
          ?mouseMove:('a -> glEnv ref -> 'a) ->
            ?mouseDragged:('a -> glEnv ref -> 'a) ->
              ?mouseDown:('a -> glEnv ref -> 'a) ->
                ?mouseUp:('a -> glEnv ref -> 'a) -> unit -> unit
  end
module Stream =
  struct
    type t = (string* int)
    let empty = []
    let peekch ((str,i) : t) =
      (if i < (String.length str) then Some (str.[i]) else None : char option)
    let popch ((str,i) : t) = ((str, (i + 1)) : t)
    let peekn (str,i) len =
      if (i + len) < (String.length str)
      then Some (String.sub str i len)
      else None
    let popn (str,i) len = (str, (i + len))
    let switch stream matchstr =
      let len = String.length matchstr in
      match peekn stream len with
      | ((Some (peek))[@explicit_arity ]) when peek = matchstr ->
          popn stream len
      | ((Some (peek))[@explicit_arity ]) ->
          failwith
            ("Could not match '" ^
               (matchstr ^ ("', got '" ^ (peek ^ "' instead."))))
      | None  -> failwith ("Could not match " ^ matchstr)
    let charsRemaining (str,i) = (String.length str) - i
    let create (str : string) = ((str, 0) : t)
  end
let read (name : string) =
  let ic = open_in name in
  let try_read () = try Some (input_line ic) with | End_of_file  -> None in
  let rec loop acc =
    match try_read () with
    | ((Some (s))[@explicit_arity ]) ->
        loop ((String.make 1 '\\n') :: s :: acc)
    | None  -> (close_in ic; List.rev acc) in
  (loop []) |> (String.concat "")
let append_char (s : string) (c : char) = (s ^ (String.make 1 c) : string)
end
module Shaders
= struct
#1 "shaders.ml"
let vertexShaderSource =
  {|
  attribute vec3 aVertexPosition;
  attribute vec4 aVertexColor;
  attribute vec2 aTextureCoord;

  uniform mat4 uPMatrix;

  varying vec4 vColor;
  varying vec2 vTextureCoord;

  void main(void) {
    gl_Position = uPMatrix * vec4(aVertexPosition, 1.0);
    vColor = aVertexColor;
    vTextureCoord = aTextureCoord;
  }
|}
let fragmentShaderSource =
  {|
  varying vec4 vColor;
  uniform float uTextureFlag;
  varying vec2 vTextureCoord;

  uniform sampler2D uSampler;

  void main(void) {
    gl_FragColor = uTextureFlag * texture2D(uSampler, vTextureCoord) + (1.0 - uTextureFlag) * vColor;
  }
|}
end
module Utils
= struct
#1 "utils.ml"
open Common
module PConstants =
  struct
    let white = { r = 255; g = 255; b = 255 }
    let black = { r = 0; g = 0; b = 0 }
    let pi = 4.0 *. (atan 1.0)
    let two_pi = 2.0 *. pi
    let half_pi = 0.5 *. pi
    let quarter_pi = 0.25 *. pi
    let tau = two_pi
  end
module PUtils =
  struct
    let lookup_table: int array ref = ref [||]
    let color ~r  ~g  ~b  = ({ r; g; b } : colorT)
    let round i = floor (i +. 0.5)
    let max = max
    let min = min
    let sqrt = sqrt
    let abs = abs
    let ceil = ceil
    let exp = exp
    let log = log
    let sq x = x * x
    let rec pow a =
      function
      | 0 -> 1
      | 1 -> a
      | n ->
          let b = pow a (n / 2) in (b * b) * (if (n mod 2) = 0 then 1 else a)
    let constrain amt low high = max (min amt high) low
    let remapf value istart istop ostart ostop =
      ostart +.
        ((ostop -. ostart) *. ((value -. istart) /. (istop -. istart)))
    let remap x a b c d =
      int_of_float
        (remapf (float_of_int x) (float_of_int a) (float_of_int b)
           (float_of_int c) (float_of_int d))
    let norm value low high = remapf value low high 0. 1.
    let randomf low high = (Random.float (high -. low)) +. low
    let random low high = (Random.int (high - low)) + low
    let randomSeed seed = Random.init seed
    let randomGaussian () =
      let u1 = ref 0.0 in
      let u2 = ref 0.0 in
      while (!u1) <= min_float do
        (u1 := (Random.float 1.0); u2 := (Random.float 1.0)) done;
      (sqrt ((-2.0) *. (log (!u1)))) *. (cos (PConstants.two_pi *. (!u2)))
    let lerpf start stop amt = remapf amt 0. 1. start stop
    let lerp start stop amt =
      int_of_float (lerpf (float_of_int start) (float_of_int stop) amt)
    let dist (x1,y1) (x2,y2) =
      let dx = float_of_int (x2 - x1) in
      let dy = float_of_int (y2 - y1) in sqrt ((dx *. dx) +. (dy *. dy))
    let mag vec = dist (0, 0) vec
    let lerpColor low high amt =
      {
        r = (lerp low.r high.r amt);
        g = (lerp low.g high.g amt);
        b = (lerp low.b high.b amt)
      }
    let acos = acos
    let asin = asin
    let atan = atan
    let atan2 = atan2
    let cos = cos
    let degrees x = (180.0 /. PConstants.pi) *. x
    let radians x = (PConstants.pi /. 180.0) *. x
    let sin = sin
    let tan = tan
    let noise x y z =
      let p = !lookup_table in
      let fade t = ((t *. t) *. t) *. ((t *. ((t *. 6.0) -. 15.0)) +. 10.0) in
      let grad hash x y z =
        match hash land 15 with
        | 0 -> x +. y
        | 1 -> (-. x) +. y
        | 2 -> x -. y
        | 3 -> (-. x) -. y
        | 4 -> x +. z
        | 5 -> (-. x) +. z
        | 6 -> x -. z
        | 7 -> (-. x) -. z
        | 8 -> y +. z
        | 9 -> (-. y) +. z
        | 10 -> y -. z
        | 11 -> (-. y) -. z
        | 12 -> y +. x
        | 13 -> (-. y) +. z
        | 14 -> y -. x
        | 15 -> (-. y) -. z
        | _ -> 0.0 in
      let xi = (int_of_float x) land 255 in
      let yi = (int_of_float y) land 255 in
      let zi = (int_of_float z) land 255 in
      let xf = x -. (floor x) in
      let yf = y -. (floor y) in
      let zf = z -. (floor z) in
      let u = fade xf in
      let v = fade yf in
      let w = fade zf in
      let aaa = p.((p.((p.(xi)) + yi)) + zi) in
      let aba = p.((p.((p.(xi)) + (yi + 1))) + zi) in
      let aab = p.((p.((p.(xi)) + yi)) + (zi + 1)) in
      let abb = p.((p.((p.(xi)) + (yi + 1))) + (zi + 1)) in
      let baa = p.((p.((p.(xi + 1)) + yi)) + zi) in
      let bba = p.((p.((p.(xi + 1)) + (yi + 1))) + zi) in
      let bab = p.((p.((p.(xi + 1)) + yi)) + (zi + 1)) in
      let bbb = p.((p.((p.(xi + 1)) + (yi + 1))) + (zi + 1)) in
      let x1 = lerpf (grad aaa xf yf zf) (grad baa (xf -. 1.0) yf zf) u in
      let x2 =
        lerpf (grad aba xf (yf -. 1.0) zf)
          (grad bba (xf -. 1.0) (yf -. 1.0) zf) u in
      let y1 = lerpf x1 x2 v in
      let x1 =
        lerpf (grad aab xf yf (zf -. 1.0))
          (grad bab (xf -. 1.0) yf (zf -. 1.0)) u in
      let x2 =
        lerpf (grad abb xf (yf -. 1.0) (zf -. 1.0))
          (grad bbb (xf -. 1.0) (yf -. 1.0) (zf -. 1.0)) u in
      let y2 = lerpf x1 x2 v in ((lerpf y1 y2 w) +. 1.0) /. 2.0
    let shuffle array =
      let array = Array.copy array in
      let length = Array.length array in
      for i = 0 to 256 - 1 do
        (let j = Random.int (length - i) in
         let tmp = array.(i) in
         array.(i) <- array.(i + j); array.(i + j) <- tmp)
      done;
      array
    let noiseSeed seed =
      let state = Random.get_state () in
      Random.init seed;
      (let array = Array.make 256 0 in
       let array = Array.mapi (fun i  -> fun _  -> i) array in
       let array = shuffle array in
       let double_array = Array.append array array in
       lookup_table := double_array; Random.set_state state)
    let rec split stream sep accstr acc =
      match Stream.peekch stream with
      | ((Some (c))[@explicit_arity ]) when c = sep ->
          split (Stream.popch stream) sep "" (accstr :: acc)
      | ((Some (c))[@explicit_arity ]) ->
          split (Stream.popch stream) sep (append_char accstr c) acc
      | None  -> List.rev (accstr :: acc)
    let split str sep = split (Stream.create str) sep "" []
  end
end
module Glhelpers
= struct
#1 "glhelpers.ml"
open Common
open Glloader
open Utils
let circularBufferSize = 6 * 10000
let globalVertexArray =
  ref (Gl.Bigarray.create Gl.Bigarray.Float32 (circularBufferSize * 9))
let globalElementArray =
  ref (Gl.Bigarray.create Gl.Bigarray.Uint16 circularBufferSize)
let vertexArrayPtr = ref 0
let elementArrayPtr = ref 0
let getProgram ~gl:(gl : Gl.contextT) 
  ~vertexShader:(vertexShaderSource : string) 
  ~fragmentShader:(fragmentShaderSource : string)  =
  (let vertexShader = Gl.createShader gl Constants.vertex_shader in
   Gl.shaderSource gl vertexShader vertexShaderSource;
   Gl.compileShader gl vertexShader;
   (let compiledCorrectly =
      (Gl.getShaderParameter ~context:gl ~shader:vertexShader
         ~paramName:Gl.Compile_status)
        = 1 in
    if compiledCorrectly
    then
      let fragmentShader = Gl.createShader gl Constants.fragment_shader in
      (Gl.shaderSource gl fragmentShader fragmentShaderSource;
       Gl.compileShader gl fragmentShader;
       (let compiledCorrectly =
          (Gl.getShaderParameter ~context:gl ~shader:fragmentShader
             ~paramName:Gl.Compile_status)
            = 1 in
        if compiledCorrectly
        then
          let program = Gl.createProgram gl in
          (Gl.attachShader ~context:gl ~program ~shader:vertexShader;
           Gl.deleteShader ~context:gl ~shader:vertexShader;
           Gl.attachShader ~context:gl ~program ~shader:fragmentShader;
           Gl.deleteShader ~context:gl ~shader:fragmentShader;
           Gl.linkProgram gl program;
           (let linkedCorrectly =
              (Gl.getProgramParameter ~context:gl ~program
                 ~paramName:Gl.Link_status)
                = 1 in
            if linkedCorrectly
            then Some program
            else
              (print_endline @@
                 ("Linking error: " ^
                    (Gl.getProgramInfoLog ~context:gl ~program));
               None)))
        else
          (print_endline @@
             ("Fragment shader error: " ^
                (Gl.getShaderInfoLog ~context:gl ~shader:fragmentShader));
           None)))
    else
      (print_endline @@
         ("Vertex shader error: " ^
            (Gl.getShaderInfoLog ~context:gl ~shader:vertexShader));
       None)) : Gl.programT option)
let createCanvas window (height : int) (width : int) =
  (Gl.Window.setWindowSize ~window ~width ~height;
   (let gl = Gl.Window.getContext window in
    Gl.viewport ~context:gl ~x:(-1) ~y:(-1) ~width ~height;
    Gl.clearColor ~context:gl ~r:0. ~g:0. ~b:0. ~a:1.;
    Gl.clear ~context:gl
      ~mask:(Constants.color_buffer_bit lor Constants.depth_buffer_bit);
    (let camera = { projectionMatrix = (Gl.Mat4.create ()) } in
     let vertexBuffer = Gl.createBuffer gl in
     let elementBuffer = Gl.createBuffer gl in
     let program =
       match getProgram ~gl ~vertexShader:Shaders.vertexShaderSource
               ~fragmentShader:Shaders.fragmentShaderSource
       with
       | None  ->
           failwith
             "Could not create the program and/or the shaders. Aborting."
       | ((Some (program))[@explicit_arity ]) -> program in
     Gl.useProgram gl program;
     (let aVertexPosition =
        Gl.getAttribLocation ~context:gl ~program ~name:"aVertexPosition" in
      Gl.enableVertexAttribArray ~context:gl ~attribute:aVertexPosition;
      (let aVertexColor =
         Gl.getAttribLocation ~context:gl ~program ~name:"aVertexColor" in
       Gl.enableVertexAttribArray ~context:gl ~attribute:aVertexColor;
       (let pMatrixUniform = Gl.getUniformLocation gl program "uPMatrix" in
        Gl.uniformMatrix4fv ~context:gl ~location:pMatrixUniform
          ~value:(camera.projectionMatrix);
        (let aTextureCoord =
           Gl.getAttribLocation ~context:gl ~program ~name:"aTextureCoord" in
         Gl.enableVertexAttribArray ~context:gl ~attribute:aTextureCoord;
         (let uSampler = Gl.getUniformLocation gl program "uSampler" in
          let uTextureFlag = Gl.getUniformLocation gl program "uTextureFlag" in
          let texture = Gl.createTexture gl in
          Gl.bindTexture ~context:gl ~target:Constants.texture_2d ~texture;
          Gl.texImage2D ~context:gl ~target:Constants.texture_2d ~level:0
            ~internalFormat:Constants.rgba ~width:1 ~height:1
            ~format:Constants.rgba ~type_:Constants.unsigned_byte
            ~data:(Gl.toTextureData [|0;0;0;0|]);
          Gl.texParameteri ~context:gl ~target:Constants.texture_2d
            ~pname:Constants.texture_mag_filter ~param:Constants.linear;
          Gl.texParameteri ~context:gl ~target:Constants.texture_2d
            ~pname:Constants.texture_min_filter
            ~param:Constants.linear_mipmap_nearest;
          Gl.enable ~context:gl Constants.blend;
          Gl.blendFunc ~context:gl Constants.src_alpha
            Constants.one_minus_src_alpha;
          Gl.Mat4.ortho ~out:(camera.projectionMatrix) ~left:0.
            ~right:(float_of_int width) ~bottom:(float_of_int height) ~top:0.
            ~near:0. ~far:100.;
          (let currFill = { r = 0; g = 0; b = 0 } in
           let currBackground = { r = 0; g = 0; b = 0 } in
           {
             camera;
             window;
             gl;
             vertexBuffer;
             elementBuffer;
             aVertexPosition;
             aTextureCoord;
             aVertexColor;
             pMatrixUniform;
             uSampler;
             uTextureFlag;
             texture;
             currFill;
             currBackground;
             mouse = { pos = (0, 0); prevPos = (0, 0); pressed = false };
             stroke = { color = { r = 0; g = 0; b = 0 }; weight = 10 };
             frame = { count = 1; rate = 10 };
             size = { height; width; resizeable = true }
           })))))))) : glEnv)
let addRectToGlobalBatch (x1,y1) (x2,y2) (x3,y3) (x4,y4) { r; g; b } =
  let set = Gl.Bigarray.set in
  let toColorFloat i = (float_of_int i) /. 255. in
  let (r,g,b) = ((toColorFloat r), (toColorFloat g), (toColorFloat b)) in
  let i = !vertexArrayPtr in
  let vertexArrayToMutate = !globalVertexArray in
  set vertexArrayToMutate (i + 0) x1;
  set vertexArrayToMutate (i + 1) y1;
  set vertexArrayToMutate (i + 2) 0.0;
  set vertexArrayToMutate (i + 3) r;
  set vertexArrayToMutate (i + 4) g;
  set vertexArrayToMutate (i + 5) b;
  set vertexArrayToMutate (i + 6) 1.;
  set vertexArrayToMutate (i + 7) 0.0;
  set vertexArrayToMutate (i + 8) 0.0;
  set vertexArrayToMutate (i + 9) x2;
  set vertexArrayToMutate (i + 10) y2;
  set vertexArrayToMutate (i + 11) 0.0;
  set vertexArrayToMutate (i + 12) r;
  set vertexArrayToMutate (i + 13) g;
  set vertexArrayToMutate (i + 14) b;
  set vertexArrayToMutate (i + 15) 1.;
  set vertexArrayToMutate (i + 16) 0.0;
  set vertexArrayToMutate (i + 17) 0.0;
  set vertexArrayToMutate (i + 18) x3;
  set vertexArrayToMutate (i + 19) y3;
  set vertexArrayToMutate (i + 20) 0.0;
  set vertexArrayToMutate (i + 21) r;
  set vertexArrayToMutate (i + 22) g;
  set vertexArrayToMutate (i + 23) b;
  set vertexArrayToMutate (i + 24) 1.;
  set vertexArrayToMutate (i + 25) 0.0;
  set vertexArrayToMutate (i + 26) 0.0;
  set vertexArrayToMutate (i + 27) x4;
  set vertexArrayToMutate (i + 28) y4;
  set vertexArrayToMutate (i + 29) 0.0;
  set vertexArrayToMutate (i + 30) r;
  set vertexArrayToMutate (i + 31) g;
  set vertexArrayToMutate (i + 32) b;
  set vertexArrayToMutate (i + 33) 1.;
  set vertexArrayToMutate (i + 34) 0.0;
  set vertexArrayToMutate (i + 35) 0.0;
  vertexArrayPtr := (i + 36);
  (let ii = i / 9 in
   let j = !elementArrayPtr in
   let elementArrayToMutate = !globalElementArray in
   set elementArrayToMutate (j + 0) ii;
   set elementArrayToMutate (j + 1) (ii + 1);
   set elementArrayToMutate (j + 2) (ii + 2);
   set elementArrayToMutate (j + 3) (ii + 1);
   set elementArrayToMutate (j + 4) (ii + 2);
   set elementArrayToMutate (j + 5) (ii + 3);
   elementArrayPtr := (j + 6))
let drawGeometry
  ~vertexArray:(vertexArray : (float,Gl.Bigarray.float32_elt) Gl.Bigarray.t) 
  ~elementArray:(elementArray :
                  (int,Gl.Bigarray.int16_unsigned_elt) Gl.Bigarray.t)
   ~mode  ~count  ?(textureFlag= 0.)  ?textureBuffer  env =
  let textureBuffer =
    match textureBuffer with
    | None  -> env.texture
    | ((Some (textureBuffer))[@explicit_arity ]) -> textureBuffer in
  Gl.bindBuffer ~context:(env.gl) ~target:Constants.array_buffer
    ~buffer:(env.vertexBuffer);
  Gl.bufferData ~context:(env.gl) ~target:Constants.array_buffer
    ~data:vertexArray ~usage:Constants.stream_draw;
  Gl.vertexAttribPointer ~context:(env.gl) ~attribute:(env.aVertexPosition)
    ~size:3 ~type_:Constants.float_ ~normalize:false ~stride:(9 * 4)
    ~offset:0;
  Gl.vertexAttribPointer ~context:(env.gl) ~attribute:(env.aVertexColor)
    ~size:4 ~type_:Constants.float_ ~normalize:false ~stride:(9 * 4)
    ~offset:(3 * 4);
  Gl.vertexAttribPointer ~context:(env.gl) ~attribute:(env.aTextureCoord)
    ~size:2 ~type_:Constants.float_ ~normalize:false ~stride:(9 * 4)
    ~offset:(7 * 4);
  Gl.bindBuffer ~context:(env.gl) ~target:Constants.element_array_buffer
    ~buffer:(env.elementBuffer);
  Gl.bufferData ~context:(env.gl) ~target:Constants.element_array_buffer
    ~data:elementArray ~usage:Constants.stream_draw;
  Gl.activeTexture ~context:(env.gl) ~target:Constants.texture0;
  Gl.bindTexture ~context:(env.gl) ~target:Constants.texture_2d
    ~texture:textureBuffer;
  Gl.uniform1i ~context:(env.gl) ~location:(env.uSampler) 0;
  Gl.uniformMatrix4fv ~context:(env.gl) ~location:(env.pMatrixUniform)
    ~value:((env.camera).projectionMatrix);
  Gl.uniform1f ~context:(env.gl) ~location:(env.uTextureFlag) textureFlag;
  Gl.drawElements ~context:(env.gl) ~mode ~count
    ~type_:Constants.unsigned_short ~offset:0
let flushGlobalBatch env =
  if (!elementArrayPtr) > 0
  then
    (drawGeometry
       ~vertexArray:(Gl.Bigarray.sub (!globalVertexArray) ~offset:0
                       ~len:(!vertexArrayPtr))
       ~elementArray:(Gl.Bigarray.sub (!globalElementArray) ~offset:0
                        ~len:(!elementArrayPtr)) ~mode:Constants.triangles
       ~count:(!elementArrayPtr) (!env);
     vertexArrayPtr := 0;
     elementArrayPtr := 0)
let drawEllipseInternal env xCenterOfCircle yCenterOfCircle radx rady =
  let noOfFans = (radx + rady) * 2 in
  assert (((noOfFans - 2) * 3) < circularBufferSize);
  (let pi = 4.0 *. (atan 1.0) in
   let anglePerFan = (2. *. pi) /. (float_of_int noOfFans) in
   let radxf = float_of_int radx in
   let radyf = float_of_int rady in
   let toColorFloat i = (float_of_int i) /. 255. in
   let (r,g,b) =
     ((toColorFloat ((!env).currFill).r), (toColorFloat ((!env).currFill).g),
       (toColorFloat ((!env).currFill).b)) in
   let xCenterOfCirclef = float_of_int xCenterOfCircle in
   let yCenterOfCirclef = float_of_int yCenterOfCircle in
   if (circularBufferSize - (!elementArrayPtr)) < ((noOfFans - 2) * 3)
   then flushGlobalBatch env;
   (let verticesData = !globalVertexArray in
    let elementData = !globalElementArray in
    let set = Gl.Bigarray.set in
    let get = Gl.Bigarray.get in
    let vertexArrayOffset = !vertexArrayPtr in
    let elementArrayOffset = !elementArrayPtr in
    for i = 0 to noOfFans - 1 do
      (let angle = anglePerFan *. (float_of_int (i + 1)) in
       let xCoordinate = xCenterOfCirclef +. ((cos angle) *. radxf) in
       let yCoordinate = yCenterOfCirclef +. ((sin angle) *. radyf) in
       let ii = (i * 9) + vertexArrayOffset in
       set verticesData (ii + 0) xCoordinate;
       set verticesData (ii + 1) yCoordinate;
       set verticesData (ii + 2) 0.0;
       set verticesData (ii + 3) r;
       set verticesData (ii + 4) g;
       set verticesData (ii + 5) b;
       set verticesData (ii + 6) 1.0;
       set verticesData (ii + 7) 0.0;
       set verticesData (ii + 8) 0.0;
       if i < 3
       then set elementData (i + elementArrayOffset) (ii / 9)
       else
         (let jj = (((i - 3) * 3) + elementArrayOffset) + 3 in
          set elementData jj (vertexArrayOffset / 9);
          set elementData (jj + 1) (get elementData (jj - 1));
          set elementData (jj + 2) (ii / 9)))
    done;
    vertexArrayPtr := ((!vertexArrayPtr) + (noOfFans * 9));
    elementArrayPtr := (((!elementArrayPtr) + ((noOfFans - 3) * 3)) + 3)))
let loadImage (env : glEnv ref) filename =
  (let imageRef = ref None in
   Gl.loadImage ~filename
     ~callback:(fun imageData  ->
                  match imageData with
                  | None  ->
                      failwith ("Could not load image '" ^ (filename ^ "'."))
                  | ((Some (img))[@explicit_arity ]) ->
                      let env = !env in
                      let textureBuffer = Gl.createTexture ~context:(env.gl) in
                      let height = Gl.getImageHeight img in
                      let width = Gl.getImageWidth img in
                      (imageRef :=
                         (Some { img; textureBuffer; height; width });
                       Gl.bindTexture ~context:(env.gl)
                         ~target:Constants.texture_2d ~texture:textureBuffer;
                       Gl.texImage2DWithImage ~context:(env.gl)
                         ~target:Constants.texture_2d ~level:0 ~image:img;
                       Gl.texParameteri ~context:(env.gl)
                         ~target:Constants.texture_2d
                         ~pname:Constants.texture_mag_filter
                         ~param:Constants.linear;
                       Gl.texParameteri ~context:(env.gl)
                         ~target:Constants.texture_2d
                         ~pname:Constants.texture_min_filter
                         ~param:Constants.linear)) ();
   imageRef : imageT)
let addTextureToGlobalBatch { width; height } ~x  ~y  ~subx  ~suby  ~subw 
  ~subh  =
  let (fsubx,fsuby,fsubw,fsubh) =
    (((float_of_int subx) /. (float_of_int width)),
      ((float_of_int suby) /. (float_of_int height)),
      ((float_of_int subw) /. (float_of_int width)),
      ((float_of_int subh) /. (float_of_int height))) in
  let (x1,y1) = ((float_of_int @@ (x + subw)), (float_of_int @@ (y + subh))) in
  let (x2,y2) = ((float_of_int x), (float_of_int @@ (y + subh))) in
  let (x3,y3) = ((float_of_int @@ (x + subw)), (float_of_int y)) in
  let (x4,y4) = ((float_of_int x), (float_of_int y)) in
  let set = Gl.Bigarray.set in
  let ii = !vertexArrayPtr in
  let vertexArray = !globalVertexArray in
  set vertexArray (ii + 0) x1;
  set vertexArray (ii + 1) y1;
  set vertexArray (ii + 2) 0.0;
  set vertexArray (ii + 3) 0.0;
  set vertexArray (ii + 4) 0.0;
  set vertexArray (ii + 5) 0.0;
  set vertexArray (ii + 6) 1.;
  set vertexArray (ii + 7) (fsubx +. fsubw);
  set vertexArray (ii + 8) (fsuby +. fsubh);
  set vertexArray (ii + 9) x2;
  set vertexArray (ii + 10) y2;
  set vertexArray (ii + 11) 0.0;
  set vertexArray (ii + 12) 0.0;
  set vertexArray (ii + 13) 0.0;
  set vertexArray (ii + 14) 0.0;
  set vertexArray (ii + 15) 1.;
  set vertexArray (ii + 16) fsubx;
  set vertexArray (ii + 17) (fsuby +. fsubh);
  set vertexArray (ii + 18) x3;
  set vertexArray (ii + 19) y3;
  set vertexArray (ii + 20) 0.0;
  set vertexArray (ii + 21) 0.0;
  set vertexArray (ii + 22) 0.0;
  set vertexArray (ii + 23) 0.0;
  set vertexArray (ii + 24) 1.;
  set vertexArray (ii + 25) (fsubx +. fsubw);
  set vertexArray (ii + 26) fsuby;
  set vertexArray (ii + 27) x4;
  set vertexArray (ii + 28) y4;
  set vertexArray (ii + 29) 0.0;
  set vertexArray (ii + 30) 0.0;
  set vertexArray (ii + 31) 0.0;
  set vertexArray (ii + 32) 0.0;
  set vertexArray (ii + 33) 1.;
  set vertexArray (ii + 34) fsubx;
  set vertexArray (ii + 35) fsuby;
  vertexArrayPtr := (ii + 36);
  (let jj = !elementArrayPtr in
   let elementArray = !globalElementArray in
   set elementArray jj (ii / 9);
   set elementArray (jj + 1) ((ii / 9) + 1);
   set elementArray (jj + 2) ((ii / 9) + 2);
   set elementArray (jj + 3) ((ii / 9) + 1);
   set elementArray (jj + 4) ((ii / 9) + 2);
   set elementArray (jj + 5) ((ii / 9) + 3);
   elementArrayPtr := (jj + 6))
let flushGlobalBatchWithTexture (env : glEnv ref) { textureBuffer } =
  drawGeometry
    ~vertexArray:(Gl.Bigarray.sub (!globalVertexArray) ~offset:0
                    ~len:(!vertexArrayPtr))
    ~elementArray:(Gl.Bigarray.sub (!globalElementArray) ~offset:0
                     ~len:(!elementArrayPtr)) ~mode:Constants.triangles
    ~count:(!elementArrayPtr) ~textureFlag:1.0 ~textureBuffer (!env);
  vertexArrayPtr := 0;
  elementArrayPtr := 0
let drawImageInternal (env : glEnv ref) { img; textureBuffer; width; height }
  ~x  ~y  ~subx  ~suby  ~subw  ~subh  =
  let env = !env in
  let (fsubx,fsuby,fsubw,fsubh) =
    (((float_of_int subx) /. (float_of_int width)),
      ((float_of_int suby) /. (float_of_int height)),
      ((float_of_int subw) /. (float_of_int width)),
      ((float_of_int subh) /. (float_of_int height))) in
  let (x1,y1) = ((float_of_int @@ (x + subw)), (float_of_int @@ (y + subh))) in
  let (x2,y2) = ((float_of_int x), (float_of_int @@ (y + subh))) in
  let (x3,y3) = ((float_of_int @@ (x + subw)), (float_of_int y)) in
  let (x4,y4) = ((float_of_int x), (float_of_int y)) in
  let verticesColorAndTexture =
    [|x1;y1;0.0;0.0;0.0;0.0;1.;(fsubx +. fsubw);(fsuby +. fsubh);x2;y2;0.0;0.0;0.0;0.0;1.;fsubx;(
      fsuby +. fsubh);x3;y3;0.0;0.0;0.0;0.0;1.;(fsubx +. fsubw);fsuby;x4;y4;0.0;0.0;0.0;0.0;1.;fsubx;fsuby|] in
  drawGeometry
    ~vertexArray:(Gl.Bigarray.of_array Gl.Bigarray.Float32
                    verticesColorAndTexture)
    ~elementArray:(Gl.Bigarray.of_array Gl.Bigarray.Uint16 [|0;1;2;1;2;3|])
    ~mode:Constants.triangles ~count:6 ~textureFlag:1.0 ~textureBuffer env
let resetSize env width height =
  env := { (!env) with size = { ((!env).size) with width; height } };
  Gl.viewport ~context:((!env).gl) ~x:0 ~y:0 ~width ~height;
  Gl.clearColor ~context:((!env).gl) ~r:0. ~g:0. ~b:0. ~a:1.;
  Gl.Mat4.ortho ~out:(((!env).camera).projectionMatrix) ~left:0.
    ~right:(float_of_int width) ~bottom:(float_of_int height) ~top:0.
    ~near:0. ~far:100.;
  Gl.uniformMatrix4fv ~context:((!env).gl) ~location:((!env).pMatrixUniform)
    ~value:(((!env).camera).projectionMatrix)
end
module Font
= struct
#1 "font.ml"
open Common
open Glloader
open Glhelpers
open Utils
module Font =
  struct
    module IntMap = Map.Make(struct type t = int
                                    let compare = compare end)
    module IntPairMap =
      Map.Make(struct
                 type t = (int* int)
                 let compare (a1,a2) (b1,b2) =
                   let first = compare a1 b1 in
                   if first <> 0 then first else compare a1 b2
               end)
    type charT =
      {
      x: int;
      y: int;
      width: int;
      height: int;
      xoffset: int;
      yoffset: int;
      xadvance: int;}
    type t = {
      chars: charT IntMap.t;
      kerning: int IntPairMap.t;
      image: imageT;}
    let rec parse_num (stream : Stream.t) acc =
      (match Stream.peekch stream with
       | ((Some (('-' as c)))[@explicit_arity ])|((Some
         (('0'..'9' as c)))[@explicit_arity ]) ->
           parse_num (Stream.popch stream) (append_char acc c)
       | _ ->
           (try (stream, (int_of_string acc))
            with | _ -> failwith ("Could not parse number [" ^ (acc ^ "]."))) : 
      (Stream.t* int))
    let parse_num stream = parse_num stream ""
    let rec parse_string (stream : Stream.t) (acc : string) =
      (match Stream.peekch stream with
       | ((Some ('"'))[@explicit_arity ]) -> ((Stream.popch stream), acc)
       | ((Some (c))[@explicit_arity ]) ->
           parse_string (Stream.popch stream) (append_char acc c)
       | None  -> failwith "Unterminated string." : (Stream.t* string))
    let parse_string stream = parse_string stream ""
    let rec pop_line stream =
      match Stream.peekch stream with
      | ((Some ('\\n'))[@explicit_arity ]) -> Stream.popch stream
      | ((Some (c))[@explicit_arity ]) -> pop_line (Stream.popch stream)
      | None  -> failwith "could not pop line"
    let rec parse_char_fmt stream num map =
      if num < 0
      then (stream, map)
      else
        (let stream = Stream.switch stream "char id=" in
         let (stream,char_id) = parse_num stream in
         let stream = Stream.switch stream " x=" in
         let (stream,x) = parse_num stream in
         let stream = Stream.switch stream " y=" in
         let (stream,y) = parse_num stream in
         let stream = Stream.switch stream " width=" in
         let (stream,width) = parse_num stream in
         let stream = Stream.switch stream " height=" in
         let (stream,height) = parse_num stream in
         let stream = Stream.switch stream " xoffset=" in
         let (stream,xoffset) = parse_num stream in
         let stream = Stream.switch stream " yoffset=" in
         let (stream,yoffset) = parse_num stream in
         let stream = Stream.switch stream " xadvance=" in
         let (stream,xadvance) = parse_num stream in
         let stream = pop_line stream in
         let new_map =
           IntMap.add char_id
             { x; y; width; height; xoffset; yoffset; xadvance } map in
         parse_char_fmt stream (num - 1) new_map)
    let rec parse_kern_fmt stream num map =
      if num = 0
      then (stream, map)
      else
        (let stream = Stream.switch stream "kerning first=" in
         let (stream,first) = parse_num stream in
         let stream = Stream.switch stream " second=" in
         let (stream,second) = parse_num stream in
         let stream = Stream.switch stream " amount=" in
         let (stream,amount) = parse_num stream in
         let stream = pop_line stream in
         let new_map = IntPairMap.add (first, second) amount map in
         parse_kern_fmt stream (num - 1) new_map)
    let replaceFilename path filename =
      let splitStr = PUtils.split path '/' in
      let revLst = List.rev splitStr in
      let newRevLst = match revLst with | hd::tl -> filename :: tl | [] -> [] in
      let newLst = List.rev newRevLst in String.concat "/" newLst
    let parseFontFormat env path =
      let stream = (read path) |> Stream.create in
      let stream = (stream |> pop_line) |> pop_line in
      let stream = Stream.switch stream "page id=0 file=\\"" in
      let (stream,filename) = parse_string stream in
      let stream = pop_line stream in
      let stream = Stream.switch stream "chars count=" in
      let (stream,num_chars) = parse_num stream in
      let stream = pop_line stream in
      let (stream,char_map) = parse_char_fmt stream num_chars IntMap.empty in
      let stream = Stream.switch stream "kernings count=" in
      let (stream,num_kerns) = parse_num stream in
      let stream = pop_line stream in
      let (_,kern_map) = parse_kern_fmt stream num_kerns IntPairMap.empty in
      let img_filename = replaceFilename path filename in
      print_endline path;
      print_endline img_filename;
      {
        chars = char_map;
        kerning = kern_map;
        image = (loadImage env img_filename)
      }
    let getChar fnt ch =
      let code = Char.code ch in
      try IntMap.find code fnt.chars
      with
      | _ ->
          failwith
            ("Could not find character " ^
               ((string_of_int code) ^ " in font."))
    let drawChar env fnt image (ch : char) (last : char option) x y =
      let c = getChar fnt ch in
      let kernAmount =
        match last with
        | ((Some (lastCh))[@explicit_arity ]) ->
            let first = Char.code lastCh in
            let second = Char.code ch in
            (try IntPairMap.find (first, second) fnt.kerning with | _ -> 0)
        | None  -> 0 in
      match image with
      | ((Some (img))[@explicit_arity ]) ->
          (addTextureToGlobalBatch img ~x:((x + c.xoffset) + kernAmount)
             ~y:(y + c.yoffset) ~subx:(c.x) ~suby:(c.y) ~subw:(c.width)
             ~subh:(c.height);
           c.xadvance + kernAmount)
      | None  -> c.xadvance + kernAmount
    let drawString env fnt (str : string) x y =
      match !(fnt.image) with
      | ((Some (img))[@explicit_arity ]) ->
          let offset = ref x in
          let lastChar = ref None in
          (flushGlobalBatch env;
           String.iter
             (fun c  ->
                let advance =
                  drawChar env fnt (Some img) c (!lastChar) (!offset) y in
                offset := ((!offset) + advance); lastChar := (Some c)) str;
           flushGlobalBatchWithTexture env img)
      | None  -> print_endline "loading font."
  end
end
module Drawfunctions
= struct
#1 "drawfunctions.ml"
open Common
open Glloader
open Glhelpers
open Utils
open Font
module P =
  struct
    let width env = ((!env).size).width
    let height env = ((!env).size).height
    let mouse env = ((!env).mouse).pos
    let pmouse env = ((!env).mouse).prevPos
    let mousePressed env = ((!env).mouse).pressed
    let background (env : glEnv ref) (c : colorT) =
      env := { (!env) with currBackground = c }
    let fill (env : glEnv ref) (c : colorT) =
      env := { (!env) with currFill = c }
    let frameRate (env : glEnv ref) = ((!env).frame).rate
    let frameCount (env : glEnv ref) = ((!env).frame).count
    let size (env : glEnv ref) width height =
      Gl.Window.setWindowSize ~window:((!env).window) ~width ~height;
      resetSize env width height
    let rect (env : glEnv ref) x y width height =
      if (!elementArrayPtr) == circularBufferSize then flushGlobalBatch env;
      addRectToGlobalBatch
        ((float_of_int @@ (x + width)), (float_of_int @@ (y + height)))
        ((float_of_int x), (float_of_int @@ (y + height)))
        ((float_of_int @@ (x + width)), (float_of_int y))
        ((float_of_int x), (float_of_int y)) (!env).currFill
    let resizeable (env : glEnv ref) resizeable =
      env := { (!env) with size = { ((!env).size) with resizeable } }
    let rectf (env : glEnv ref) x y width height =
      if (!elementArrayPtr) == circularBufferSize then flushGlobalBatch env;
      addRectToGlobalBatch ((x +. width), (y +. height)) (x, (y +. height))
        ((x +. width), y) (x, y) (!env).currFill
    let loadImage = loadImage
    let image (env : glEnv ref) img x y =
      match !img with
      | None  -> print_endline "image not ready yet, just doing nothing :D"
      | ((Some ({ img; textureBuffer; width; height }))[@explicit_arity ]) ->
          drawImageInternal env { img; textureBuffer; width; height } x y 0 0
            width height
    let background env color =
      let w = width env in
      let h = height env in
      let oldEnv = !env in fill env color; rect env 0 0 w h; env := oldEnv
    let clear env =
      Gl.clear (!env).gl
        (Constants.color_buffer_bit lor Constants.depth_buffer_bit)
    let stroke env color =
      env := { (!env) with stroke = { ((!env).stroke) with color } }
    let strokeWeight env weight =
      env := { (!env) with stroke = { ((!env).stroke) with weight } }
    let line env (xx1,yy1) (xx2,yy2) =
      let dx = xx2 - xx1 in
      let dy = yy2 - yy1 in
      let mag = PUtils.dist (xx1, yy1) (xx2, yy2) in
      let radius = (float_of_int ((!env).stroke).weight) /. 2. in
      let xthing = PUtils.round (((float_of_int dy) /. mag) *. radius) in
      let ything = PUtils.round (((-. (float_of_int dx)) /. mag) *. radius) in
      let x1 = (float_of_int xx1) +. xthing in
      let y1 = (float_of_int yy1) +. ything in
      let x2 = (float_of_int xx2) +. xthing in
      let y2 = (float_of_int yy2) +. ything in
      let x3 = (float_of_int xx2) -. xthing in
      let y3 = (float_of_int yy2) -. ything in
      let x4 = (float_of_int xx1) -. xthing in
      let y4 = (float_of_int yy1) -. ything in
      addRectToGlobalBatch (x1, y1) (x2, y2) (x3, y3) (x4, y4)
        (!env).currFill;
      if (!elementArrayPtr) == circularBufferSize then flushGlobalBatch env
    let ellipse env a b c d = drawEllipseInternal env a b c d
    let loadFont env filename = Font.parseFontFormat env filename
    let text env fnt str x y = Font.drawString env fnt str x y
  end
end
module Reprocessing
= struct
#1 "reprocessing.ml"
open Common
open Glloader
open Glhelpers
open Utils
module PUtils = PUtils
module PConstants = PConstants
module P = Drawfunctions.P
type 'a userCallbackT = 'a -> glState ref -> ('a* glState)
let afterDraw f (env : glEnv ref) =
  let rate = int_of_float (1000. /. f) in
  env :=
    {
      (!env) with
      mouse = { ((!env).mouse) with prevPos = (((!env).mouse).pos) };
      frame = { count = (((!env).frame).count + 1); rate }
    };
  flushGlobalBatch env

external removeFromDOM : 'a -> unit = "remove" [@@bs.send]
external setID : 'a -> string -> unit = "id" [@@bs.set]
external appendChild : 'a -> 'b -> unit = "appendChild" [@@bs.send]

let canvas = (Gl.Window.initWithCanvasID "main-canvas");
module ReProcessor : ReProcessorT =
  struct
    type t = glEnv ref
    let run ~setup  ?draw  ?mouseMove  ?mouseDragged  ?mouseDown  ?mouseUp 
      () =
      Random.self_init ();
      PUtils.noiseSeed (Random.int ((PUtils.pow 2 30) - 1));
      removeFromDOM (Document.getElementById "main-canvas");
      let canvas = createElement "canvas" in
      setBackgroundColor (getStyle canvas) "black";
      setID canvas "main-canvas";
      appendChild (Document.getElementById "canvas-wrapper") canvas;
      (
        let env = ref (createCanvas canvas 200 200) in
        let userState = ref (setup env) in
        let reDrawPreviousBufferOnSecondFrame =
         let width = Gl.Window.getWidth (!env).window in
         let height = Gl.Window.getHeight (!env).window in
         let data =
           Gl.readPixelsRGBA ~context:((!env).gl) ~x:0 ~y:0 ~width ~height in
         let textureBuffer = Gl.createTexture ~context:((!env).gl) in
         Gl.bindTexture ~context:((!env).gl) ~target:Constants.texture_2d
           ~texture:textureBuffer;
         Gl.texImage2D ~context:((!env).gl) ~target:Constants.texture_2d
           ~level:0 ~internalFormat:Constants.rgba ~width ~height
           ~format:Constants.rgba ~type_:Constants.unsigned_byte ~data;
         Gl.texParameteri ~context:((!env).gl) ~target:Constants.texture_2d
           ~pname:Constants.texture_mag_filter ~param:Constants.linear;
         Gl.texParameteri ~context:((!env).gl) ~target:Constants.texture_2d
           ~pname:Constants.texture_min_filter ~param:Constants.linear;
         Gl.texParameteri ~context:((!env).gl) ~target:Constants.texture_2d
           ~pname:Constants.texture_wrap_s ~param:Constants.clamp_to_edge;
         Gl.texParameteri ~context:((!env).gl) ~target:Constants.texture_2d
           ~pname:Constants.texture_wrap_t ~param:Constants.clamp_to_edge;
         (fun ()  ->
            let (x,y) = (0, 0) in
            let (x1,y1) =
              ((float_of_int @@ (x + width)), (float_of_int @@ y)) in
            let (x2,y2) = ((float_of_int x), (float_of_int @@ y)) in
            let (x3,y3) =
              ((float_of_int @@ (x + width)), (float_of_int @@ (y + height))) in
            let (x4,y4) = ((float_of_int x), (float_of_int @@ (y + height))) in
            let verticesColorAndTexture =
              [|x1;y1;0.0;0.0;0.0;0.0;1.;1.0;1.0;x2;y2;0.0;0.0;0.0;0.0;1.;0.0;1.0;x3;y3;0.0;0.0;0.0;0.0;1.;1.0;0.0;x4;y4;0.0;0.0;0.0;0.0;1.;0.0;0.0|] in
            drawGeometry
              ~vertexArray:(Gl.Bigarray.of_array Gl.Bigarray.Float32
                              verticesColorAndTexture)
              ~elementArray:(Gl.Bigarray.of_array Gl.Bigarray.Uint16
                               [|0;1;2;1;2;3|]) ~mode:Constants.triangles
              ~count:6 ~textureFlag:1.0 ~textureBuffer (!env)) in
       Gl.render ~window:((!env).window)
         ~displayFunc:(fun f  ->
                         if ((!env).frame).count == 2
                         then reDrawPreviousBufferOnSecondFrame ();
                         (match draw with
                          | ((Some (draw))[@explicit_arity ]) ->
                              userState := (draw (!userState) env)
                          | None  -> ());
                         afterDraw f env)
         ~mouseDown:(fun ~button  ->
                       fun ~state  ->
                         fun ~x  ->
                           fun ~y  ->
                             env :=
                               {
                                 (!env) with
                                 mouse =
                                   {
                                     ((!env).mouse) with
                                     pos = (x, y);
                                     pressed = true
                                   }
                               };
                             (match mouseDown with
                              | ((Some (mouseDown))[@explicit_arity ]) ->
                                  userState := (mouseDown (!userState) env)
                              | None  -> ()))
         ~mouseUp:(fun ~button  ->
                     fun ~state  ->
                       fun ~x  ->
                         fun ~y  ->
                           env :=
                             {
                               (!env) with
                               mouse =
                                 {
                                   ((!env).mouse) with
                                   pos = (x, y);
                                   pressed = false
                                 }
                             };
                           (match mouseUp with
                            | ((Some (mouseUp))[@explicit_arity ]) ->
                                userState := (mouseUp (!userState) env)
                            | None  -> ()))
         ~mouseMove:(fun ~x  ->
                       fun ~y  ->
                         env :=
                           {
                             (!env) with
                             mouse = { ((!env).mouse) with pos = (x, y) }
                           };
                         if ((!env).mouse).pressed
                         then
                           (match mouseDragged with
                            | ((Some (mouseDragged))[@explicit_arity ]) ->
                                userState := (mouseDragged (!userState) env)
                            | None  -> ())
                         else
                           (match mouseMove with
                            | ((Some (mouseMove))[@explicit_arity ]) ->
                                userState := (mouseMove (!userState) env)
                            | None  -> ()))
         ~windowResize:(fun ()  ->
                          if ((!env).size).resizeable
                          then
                            let height = Gl.Window.getHeight (!env).window in
                            let width = Gl.Window.getWidth (!env).window in
                            resetSize env width height
                          else P.size env (P.width env) (P.height env)) ())
  end 
end

`;

// var myWorker = new Worker("worker.js");
// 
// myWorker.onmessage = function(e) {
//   let rsp = JSON.parse(e.data);
//   console.log(rsp);
//   if (rsp.js_code !== undefined) {
//      evalCode(rsp.js_code);
//   }
// };
// 
// function runCompiler() {
//   myWorker.postMessage(predefinedStuff + document.refmt(myCode1Mirror.getValue()).c);
// }

function onEditChanges(cm, change) {
  if(typeof compile_code === 'undefined'){
    console.log('init....');
    compile_code = ocaml.compile;
  }
  console.error = redirect_err;
  var raw = compile_code(predefinedStuff + document.refmt(myCode1Mirror.getValue()).c);
  errorMirror.setValue(get_error_output());
  console.error = original_err;
  console.log(raw);
  var rsp = JSON.parse(raw); // can we save this from parsing?
  if (rsp.js_code !== undefined) {
    // jsCode1Mirror.setValue(rsp.js_code);
    // eval
    if(shouldEval()) {
       evalCode(rsp.js_code)
    }
  } else {
    jsCode1Mirror.setValue(rsp.js_error_msg);

  }

}
// myCode2Mirror.on("changes", onEditChanges);

jsCode1Mirror.setSize(null,codeMirrorDefaultHeight);



//checks or unchecks the eval button
function changeEvalButton(bool) {
  $('#option-eval').prop('checked', bool);
  onEvalButtonChange();
}

//creates a gist from OCaml code
$('#share').click(function (e) {
  var state = $(this).button('loading');
  var request =
  {
    "description": "BuckleScript Gist",
    "public": true,
    "files": {
      "gist.ml": {
        "content": myCode1Mirror.getValue()
      }
    }
  };

  $
    .ajax({ url:'https://api.github.com/gists',
            type: 'POST',
            data: JSON.stringify(request)
          })
    .done(function (response) {
      state.button('reset');
      $('#shareModal').modal('show');
      var url = 'https://bloomberg.github.io/bucklescript/js-demo/?gist=' + response.id;
      $('#shareModalBody').html('<a href=' + '"' + url + '"' + 'target="_blank"' + '>' + url + '</a>');
    })
    .error(function (err) {
      state.button('reset');
      $('#shareModal').modal('show');
      $('#shareModalBody').text('Sorry! Currently GitHub\'s API limits the number of requests we can send per hour. Please try again later.');
    })
});

//copy link to clipboard
var copy = new Clipboard('#copyButton');
copy.on('success', function(e) {
  e.clearSelection();
  $('#copyGlyph').attr('class', 'glyphicon glyphicon-ok');
});

//reset clipboard icon when modal is closed
$('#shareModal').on('hidden.bs.modal', function (e) {
  $('#copyGlyph').attr('class', 'glyphicon glyphicon-copy');
});
