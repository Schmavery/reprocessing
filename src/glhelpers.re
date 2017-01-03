/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
open Common;

open Glloader;

open Utils;

let getProgram
    gl::(gl: Gl.contextT)
    vertexShader::(vertexShaderSource: string)
    fragmentShader::(fragmentShaderSource: string)
    :option Gl.programT => {
  let vertexShader = Gl.createShader gl Constants.vertex_shader;
  Gl.shaderSource gl vertexShader vertexShaderSource;
  Gl.compileShader gl vertexShader;
  let compiledCorrectly =
    Gl.getShaderParameter context::gl shader::vertexShader paramName::Gl.Compile_status == 1;
  if compiledCorrectly {
    let fragmentShader = Gl.createShader gl Constants.fragment_shader;
    Gl.shaderSource gl fragmentShader fragmentShaderSource;
    Gl.compileShader gl fragmentShader;
    let compiledCorrectly =
      Gl.getShaderParameter context::gl shader::fragmentShader paramName::Gl.Compile_status == 1;
    if compiledCorrectly {
      let program = Gl.createProgram gl;
      Gl.attachShader context::gl ::program shader::vertexShader;
      Gl.deleteShader context::gl shader::vertexShader;
      Gl.attachShader context::gl ::program shader::fragmentShader;
      Gl.deleteShader context::gl shader::fragmentShader;
      Gl.linkProgram gl program;
      let linkedCorrectly =
        Gl.getProgramParameter context::gl ::program paramName::Gl.Link_status == 1;
      if linkedCorrectly {
        Some program
      } else {
        print_endline @@ "Linking error: " ^ Gl.getProgramInfoLog context::gl ::program;
        None
      }
    } else {
      print_endline @@
      "Fragment shader error: " ^ Gl.getShaderInfoLog context::gl shader::fragmentShader;
      None
    }
  } else {
    print_endline @@
    "Vertex shader error: " ^ Gl.getShaderInfoLog context::gl shader::vertexShader;
    None
  }
};

let createCanvas window (height: int) (width: int) :glEnv => {
  Gl.Window.setWindowSize ::window ::width ::height;
  let gl = Gl.Window.getContext window;
  Gl.viewport context::gl x::(-1) y::(-1) ::width ::height;
  Gl.clearColor context::gl r::0. g::0. b::0. a::1.;
  Gl.clear context::gl mask::(Constants.color_buffer_bit lor Constants.depth_buffer_bit);

  /** Camera is a simple record containing one matrix used to project a point in 3D onto the screen. **/
  let camera = {projectionMatrix: Gl.Mat4.create ()};
  let vertexBuffer = Gl.createBuffer gl;
  let program =
    switch (
      getProgram
        ::gl vertexShader::Shaders.vertexShaderSource fragmentShader::Shaders.fragmentShaderSource
    ) {
    | None => failwith "Could not create the program and/or the shaders. Aborting."
    | Some program => program
    };
  Gl.useProgram gl program;

  /** Get the attribs ahead of time to be used inside the render function **/
  let aVertexPosition = Gl.getAttribLocation context::gl ::program name::"aVertexPosition";
  Gl.enableVertexAttribArray context::gl attribute::aVertexPosition;
  let aVertexColor = Gl.getAttribLocation context::gl ::program name::"aVertexColor";
  Gl.enableVertexAttribArray context::gl attribute::aVertexColor;
  let pMatrixUniform = Gl.getUniformLocation gl program "uPMatrix";
  Gl.uniformMatrix4fv context::gl location::pMatrixUniform value::camera.projectionMatrix;

  /** Get attribute and uniform locations for later usage in the draw code. **/
  let aTextureCoord = Gl.getAttribLocation context::gl ::program name::"aTextureCoord";
  Gl.enableVertexAttribArray context::gl attribute::aTextureCoord;
  let uSampler = Gl.getUniformLocation gl program "uSampler";
  let uTextureFlag = Gl.getUniformLocation gl program "uTextureFlag";

  /** Generate texture buffer that we'll use to pass image data around. **/
  let texture = Gl.createTexture gl;

  /** Bind `texture` to `texture_2d` to modify it's magnification and minification params. **/
  Gl.bindTexture context::gl target::Constants.texture_2d ::texture;
  Gl.texParameteri
    context::gl
    target::Constants.texture_2d
    pname::Constants.texture_mag_filter
    param::Constants.linear;
  Gl.texParameteri
    context::gl
    target::Constants.texture_2d
    pname::Constants.texture_min_filter
    param::Constants.linear_mipmap_nearest;

  /** Enable blend and tell OpenGL how to blend. */
  Gl.enable context::gl Constants.blend;
  Gl.blendFunc context::gl Constants.src_alpha Constants.one_minus_src_alpha;

  /**
   * Will mutate the projectionMatrix to be an ortho matrix with the given boundaries.
   * See this link for quick explanation of what this is.
   * https://shearer12345.github.io/graphics/assets/projectionPerspectiveVSOrthographic.png
   */
  Gl.Mat4.ortho
    out::camera.projectionMatrix
    left::0.
    right::(float_of_int width)
    bottom::(float_of_int height)
    top::0.
    near::0.
    far::100.;
  let currFill = {r: 0, g: 0, b: 0};
  let currBackground = {r: 0, g: 0, b: 0};
  {
    camera,
    window,
    gl,
    vertexBuffer,
    aVertexPosition,
    aTextureCoord,
    aVertexColor,
    pMatrixUniform,
    uSampler,
    uTextureFlag,
    texture,
    currFill,
    currBackground,
    mouse: {pos: (0, 0), prevPos: (0, 0), pressed: false},
    stroke: {color: {r: 0, g: 0, b: 0}, weight: 10},
    frame: {count: 1, rate: 10},
    size: {height, width, resizeable: true}
  }
};

/*
 * This array packs all of the values that the shaders need: vertices, colors and texture coordinates.
 * We put them all in one as an optimization, so there are less back and forths between us and the GPU.
 *
 * The data looks like:
 *
 * |<---------  9 * 4 bytes  --------->|
 *  ------------------------------------
 * |  x  y  z  |  r  g  b  a  |  s  t  |  x2  y2  z2 |  r2  g2  b2  a2  |  s2  t2  | ....
 * ------------------------------------
 * |           |              |
 * +- offset: 0 bytes, stride: 9 * 4 bytes (because we need to move by 9*4 bytes to get to the next x)
 *             |              |
 *             +- offset: 3 * 4 bytes, stride: 9 * 4 bytes
 *                            |
 *                            +- offset: (3 + 4) * 4 bytes, stride: 9 * 4 bytes
 */
let makeRectVertexBuffer (x1, y1) (x2, y2) (x3, y3) (x4, y4) {r, g, b} => {
  let toColorFloat i => float_of_int i /. 255.;
  let (r, g, b) = (toColorFloat r, toColorFloat g, toColorFloat b);
  [|
    x1,
    y1,
    0.0,
    r,
    g,
    b,
    1.,
    0.0,
    0.0,
    x2,
    y2,
    0.0,
    r,
    g,
    b,
    1.,
    0.0,
    0.0,
    x3,
    y3,
    0.0,
    r,
    g,
    b,
    1.,
    0.0,
    0.0,
    x4,
    y4,
    0.0,
    r,
    g,
    b,
    1.,
    0.0,
    0.0
  |]
};

let drawVertexBuffer vertexBuffer::verticesColorAndTextureCoord ::mode ::count ::textureFlag=0. env => {
  /* Bind `vertexBuffer`, a pointer to chunk of memory to be sent to the GPU to the "register" called
     `array_buffer` */
  Gl.bindBuffer context::env.gl target::Constants.array_buffer buffer::env.vertexBuffer;

  /** Copy all of the data over into whatever's in `array_buffer` (so here it's `vertexBuffer`) **/
  Gl.bufferData
    context::env.gl
    target::Constants.array_buffer
    data::(Gl.Float32 verticesColorAndTextureCoord)
    /* TODO: Update to be GL_STREAM_DRAW but we need the constants in reglinterface */
    usage::Constants.static_draw;

  /** Tell the GPU about the shader attribute called `aVertexPosition` so it can access the data per vertex */
  Gl.vertexAttribPointer
    context::env.gl
    attribute::env.aVertexPosition
    size::3
    type_::Constants.float_
    normalize::false
    stride::(9 * 4)
    offset::0;

  /** Same as above but for `aVertexColor` **/
  Gl.vertexAttribPointer
    context::env.gl
    attribute::env.aVertexColor
    size::4
    type_::Constants.float_
    normalize::false
    stride::(9 * 4)
    offset::(3 * 4);

  /** Same as above but for `aTextureCoord` **/
  Gl.vertexAttribPointer
    context::env.gl
    attribute::env.aTextureCoord
    size::2
    type_::Constants.float_
    normalize::false
    stride::(9 * 4)
    offset::(7 * 4);

  /** This tells OpenGL that we're going to be using texture0. OpenGL imposes a limit on the number of
      texture we can manipulate at the same time. That limit depends on the device. We don't care, as we'll just
      always use texture0. **/
  Gl.activeTexture context::env.gl target::Constants.texture0;

  /** We bind `texture` to texture_2d, like we did for the vertex buffers in some ways (I think?) **/
  Gl.bindTexture context::env.gl target::Constants.texture_2d texture::env.texture;

  /** Tell OpenGL about what the uniform called `uSampler` is pointing at, here it's given 0 which is what
      texture0 represent. **/
  Gl.uniform1i context::env.gl location::env.uSampler 0;

  /** Tell OpenGL about what the uniform called `pMatrixUniform` is, here it's the projectionMatrix. **/
  Gl.uniformMatrix4fv
    context::env.gl location::env.pMatrixUniform value::env.camera.projectionMatrix;

  /** Last uniform, the `uTextureFlag` which allows us to only have one shader and flip flop between using
      a color or a texture to fill the geometry. **/
  Gl.uniform1f context::env.gl location::env.uTextureFlag textureFlag;

  /** Final call which actually does the "draw" **/
  Gl.drawArrays context::env.gl ::mode first::0 ::count
};

let drawEllipseInternal env xCenterOfCircle yCenterOfCircle radx rady => {
  let noOfFans = max radx rady * 3;
  let pi = 4.0 *. atan 1.0;
  let anglePerFan = 2. *. pi /. float_of_int noOfFans;
  let radxf = float_of_int radx;
  let radyf = float_of_int rady;
  let verticesData = ref [];
  let toColorFloat i => float_of_int i /. 255.;
  let (r, g, b) = (
    toColorFloat env.currFill.r,
    toColorFloat env.currFill.g,
    toColorFloat env.currFill.b
  );
  for i in 0 to (noOfFans - 1) {
    let angle = anglePerFan *. float_of_int (i + 1);
    let xCoordinate = float_of_int xCenterOfCircle +. cos angle *. radxf;
    let yCoordinate = float_of_int yCenterOfCircle +. sin angle *. radyf;
    /* Data format is the same as the one described in `drawRectInternal` but reversed */
    verticesData := [0.0, 0.0, 1.0, b, g, r, 0., yCoordinate, xCoordinate, ...!verticesData]
  };
  verticesData := [
    0.0,
    0.0,
    1.0,
    b,
    g,
    r,
    0.,
    float_of_int yCenterOfCircle,
    float_of_int xCenterOfCircle,
    ...!verticesData
  ];
  let verticesArray = Array.of_list (List.rev !verticesData);
  drawVertexBuffer
    vertexBuffer::verticesArray mode::Constants.triangle_fan count::(noOfFans + 1) env
};

let resetSize env width height => {
  env := {...!env, size: {...(!env).size, width, height}};
  Gl.viewport context::(!env).gl x::0 y::0 ::width ::height;
  Gl.clearColor context::(!env).gl r::0. g::0. b::0. a::1.;
  Gl.Mat4.ortho
    out::(!env).camera.projectionMatrix
    left::0.
    right::(float_of_int width)
    bottom::(float_of_int height)
    top::0.
    near::0.
    far::100.;
  Gl.uniformMatrix4fv
    context::(!env).gl location::(!env).pMatrixUniform value::(!env).camera.projectionMatrix
};
