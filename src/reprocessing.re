/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
include [%matchenv
  switch GL_BACKEND {
  | "web" => Reglweb.Webgl
  | "native" => Reglnative.Opengl
  }
];

module Constants = Reglinterface.Constants;

type glState = Gl.Window.t;

type glCamera = {projectionMatrix: Gl.Mat4.t};

type color = {r: int, b: int, g: int};

type strokeT = {color: color, weight: int};

type mouseT = {pos: (int, int), prevPos: (int, int), pressed: bool};

type frameT = {count: int, rate: int};

type sizeT = {height: int, width: int, resizeable: bool};

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

let vertexShaderSource = {|
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
|};

let fragmentShaderSource = {|
  varying vec4 vColor;
  uniform float uTextureFlag;
  varying vec2 vTextureCoord;

  uniform sampler2D uSampler;

  void main(void) {
    gl_FragColor = uTextureFlag * texture2D(uSampler, vTextureCoord) + (1.0 - uTextureFlag) * vColor;
  }
|};

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
    switch (getProgram ::gl vertexShader::vertexShaderSource fragmentShader::fragmentShaderSource) {
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

module PConstants = {
  let white = {r: 255, g: 255, b: 255};
  let black = {r: 0, g: 0, b: 0};
  let pi = 4.0 *. atan 1.0;
  let two_pi = 2.0 *. pi;
  let half_pi = 0.5 *. pi;
  let quarter_pi = 0.25 *. pi;
  let tau = two_pi;
};

module PUtils = {
  let lookup_table: ref (array int) = ref [||];
  let color ::r ::g ::b :color => {r, g, b};
  /*Calculation Functions*/
  let round i => floor (i +. 0.5);
  let max = max;
  let min = min;
  let sqrt = sqrt;
  let abs = abs;
  let ceil = ceil;
  let exp = exp;
  let log = log;
  let sq x => x * x;
  let rec pow a =>
    fun
    | 0 => 1
    | 1 => a
    | n => {
        let b = pow a (n / 2);
        b * b * (
          if (n mod 2 == 0) {
            1
          } else {
            a
          }
        )
      };
  let constrain amt low high => max (min amt high) low;
  let remapf value istart istop ostart ostop =>
    ostart +. (ostop -. ostart) *. ((value -. istart) /. (istop -. istart));
  let remap x a b c d =>
    int_of_float (
      remapf (float_of_int x) (float_of_int a) (float_of_int b) (float_of_int c) (float_of_int d)
    );
  let norm value low high => remapf value low high 0. 1.;
  let randomf low high => Random.float (high -. low) +. low;
  let random low high => Random.int (high - low) + low;
  let randomSeed seed => Random.init seed;
  let randomGaussian () => {
    let u1 = ref 0.0;
    let u2 = ref 0.0;
    while (!u1 <= min_float) {
      u1 := Random.float 1.0;
      u2 := Random.float 1.0
    };
    sqrt ((-2.0) *. log !u1) *. cos (PConstants.two_pi *. !u2)
  };
  let lerpf start stop amt => remapf amt 0. 1. start stop;
  let lerp start stop amt => int_of_float (lerpf (float_of_int start) (float_of_int stop) amt);
  let dist (x1, y1) (x2, y2) => {
    let dx = float_of_int (x2 - x1);
    let dy = float_of_int (y2 - y1);
    sqrt (dx *. dx +. dy *. dy)
  };
  let mag vec => dist (0, 0) vec;
  let lerpColor low high amt => {
    r: lerp low.r high.r amt,
    g: lerp low.g high.g amt,
    b: lerp low.b high.b amt
  };
  let acos = acos;
  let asin = asin;
  let atan = atan;
  let atan2 = atan2;
  let cos = cos;
  let degrees x => 180.0 /. PConstants.pi *. x;
  let radians x => PConstants.pi /. 180.0 *. x;
  let sin = sin;
  let tan = tan;
  let noise x y z => {
    let p = !lookup_table;
    let fade t => t *. t *. t *. (t *. (t *. 6.0 -. 15.0) +. 10.0);
    let grad hash x y z =>
      switch (hash land 15) {
      | 0 => x +. y
      | 1 => -. x +. y
      | 2 => x -. y
      | 3 => -. x -. y
      | 4 => x +. z
      | 5 => -. x +. z
      | 6 => x -. z
      | 7 => -. x -. z
      | 8 => y +. z
      | 9 => -. y +. z
      | 10 => y -. z
      | 11 => -. y -. z
      | 12 => y +. x
      | 13 => -. y +. z
      | 14 => y -. x
      | 15 => -. y -. z
      | _ => 0.0
      };
    let xi = int_of_float x land 255;
    let yi = int_of_float y land 255;
    let zi = int_of_float z land 255;
    let xf = x -. floor x;
    let yf = y -. floor y;
    let zf = z -. floor z;
    let u = fade xf;
    let v = fade yf;
    let w = fade zf;
    let aaa = p.(p.(p.(xi) + yi) + zi);
    let aba = p.(p.(p.(xi) + (yi + 1)) + zi);
    let aab = p.(p.(p.(xi) + yi) + (zi + 1));
    let abb = p.(p.(p.(xi) + (yi + 1)) + (zi + 1));
    let baa = p.(p.(p.(xi + 1) + yi) + zi);
    let bba = p.(p.(p.(xi + 1) + (yi + 1)) + zi);
    let bab = p.(p.(p.(xi + 1) + yi) + (zi + 1));
    let bbb = p.(p.(p.(xi + 1) + (yi + 1)) + (zi + 1));
    let x1 = lerpf (grad aaa xf yf zf) (grad baa (xf -. 1.0) yf zf) u;
    let x2 = lerpf (grad aba xf (yf -. 1.0) zf) (grad bba (xf -. 1.0) (yf -. 1.0) zf) u;
    let y1 = lerpf x1 x2 v;
    let x1 = lerpf (grad aab xf yf (zf -. 1.0)) (grad bab (xf -. 1.0) yf (zf -. 1.0)) u;
    let x2 =
      lerpf (grad abb xf (yf -. 1.0) (zf -. 1.0)) (grad bbb (xf -. 1.0) (yf -. 1.0) (zf -. 1.0)) u;
    let y2 = lerpf x1 x2 v;
    (lerpf y1 y2 w +. 1.0) /. 2.0
  };
  let shuffle array => {
    let array = Array.copy array;
    let length = Array.length array;
    for i in 0 to (256 - 1) {
      let j = Random.int (length - i);
      let tmp = array.(i);
      array.(i) = array.(i + j);
      array.(i + j) = tmp
    };
    array
  };
  let noiseSeed seed => {
    let state = Random.get_state ();
    Random.init seed;
    let array = Array.make 256 0;
    let array = Array.mapi (fun i _ => i) array;
    let array = shuffle array;
    let double_array = Array.append array array;
    lookup_table := double_array;
    Random.set_state state
  };
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

type imageT = ref (option Gl.imageT);

module P = {
  let width env => (!env).size.width;
  let height env => (!env).size.height;
  let mouse env => (!env).mouse.pos;
  let pmouse env => (!env).mouse.prevPos;
  let mousePressed env => (!env).mouse.pressed;
  let background (env: ref glEnv) (c: color) => env := {...!env, currBackground: c};
  let fill (env: ref glEnv) (c: color) => env := {...!env, currFill: c};
  let frameRate (env: ref glEnv) => (!env).frame.rate;
  let frameCount (env: ref glEnv) => (!env).frame.count;
  let size (env: ref glEnv) width height => {
    Gl.Window.setWindowSize window::(!env).window ::width ::height;
    resetSize env width height
  };
  let rect (env: ref glEnv) x y width height => {
    let vertexBuffer =
      makeRectVertexBuffer
        (float_of_int @@ x + width, float_of_int @@ y + height)
        (float_of_int x, float_of_int @@ y + height)
        (float_of_int @@ x + width, float_of_int y)
        (float_of_int x, float_of_int y)
        (!env).currFill;
    drawVertexBuffer ::vertexBuffer mode::Constants.triangle_strip count::4 !env
  };
  let resizeable (env: ref glEnv) resizeable =>
    env := {...!env, size: {...(!env).size, resizeable}};
  let rectf (env: ref glEnv) x y width height => {
    let vertexBuffer =
      makeRectVertexBuffer
        (x +. width, y +. height) (x, y +. height) (x +. width, y) (x, y) (!env).currFill;
    drawVertexBuffer ::vertexBuffer mode::Constants.triangle_strip count::4 !env
  };
  let loadImage (env: ref glEnv) filename :imageT => {
    let imageRef = ref None;
    Gl.loadImage
      ::filename
      callback::(
        fun imageData =>
          switch imageData {
          | None => assert false
          | Some image =>
            let env = !env;
            imageRef := Some image;
            Gl.bindTexture context::env.gl target::Constants.texture_2d texture::env.texture;
            Gl.texImage2DWithImage context::env.gl target::Constants.texture_2d level::0 ::image;
            Gl.texParameteri
              context::env.gl
              target::Constants.texture_2d
              pname::Constants.texture_mag_filter
              param::Constants.linear;
            Gl.texParameteri
              context::env.gl
              target::Constants.texture_2d
              pname::Constants.texture_min_filter
              param::Constants.linear_mipmap_nearest;
            Gl.generateMipmap context::env.gl target::Constants.texture_2d
          }
      )
      ();
    imageRef
  };
  let image (env: ref glEnv) img x y =>
    switch !img {
    | None => print_endline "image not ready yet, just doing nothing :D"
    | Some image =>
      let env = !env;
      let width = Gl.getImageWidth image;
      let height = Gl.getImageHeight image;
      let (x1, y1) = (float_of_int @@ x + width, float_of_int @@ y + height);
      let (x2, y2) = (float_of_int x, float_of_int @@ y + height);
      let (x3, y3) = (float_of_int @@ x + width, float_of_int y);
      let (x4, y4) = (float_of_int x, float_of_int y);
      let verticesColorAndTexture = [|
        x1,
        y1,
        0.0,
        0.0,
        0.0,
        0.0,
        1.,
        1.0,
        1.0,
        x2,
        y2,
        0.0,
        0.0,
        0.0,
        0.0,
        1.,
        0.0,
        1.0,
        x3,
        y3,
        0.0,
        0.0,
        0.0,
        0.0,
        1.,
        1.0,
        0.0,
        x4,
        y4,
        0.0,
        0.0,
        0.0,
        0.0,
        1.,
        0.0,
        0.0
      |];
      drawVertexBuffer
        vertexBuffer::verticesColorAndTexture
        mode::Constants.triangle_strip
        count::4
        textureFlag::1.0
        env
    };
  let background env color => {
    let w = width env;
    let h = height env;
    let oldEnv = !env;
    fill env color;
    rect env 0 0 w h;
    env := oldEnv
  };
  let clear env => Gl.clear (!env).gl (Constants.color_buffer_bit lor Constants.depth_buffer_bit);
  let stroke env color => env := {...!env, stroke: {...(!env).stroke, color}};
  let strokeWeight env weight => env := {...!env, stroke: {...(!env).stroke, weight}};
  let line env (xx1, yy1) (xx2, yy2) => {
    let dx = xx2 - xx1;
    let dy = yy2 - yy1;
    let mag = PUtils.dist (xx1, yy1) (xx2, yy2);
    let radius = float_of_int (!env).stroke.weight /. 2.;
    let xthing = PUtils.round (float_of_int dy /. mag *. radius);
    let ything = PUtils.round (-. float_of_int dx /. mag *. radius);
    let x1 = float_of_int xx1 +. xthing;
    let y1 = float_of_int yy1 +. ything;
    let x2 = float_of_int xx2 +. xthing;
    let y2 = float_of_int yy2 +. ything;
    let x3 = float_of_int xx2 -. xthing;
    let y3 = float_of_int yy2 -. ything;
    let x4 = float_of_int xx1 -. xthing;
    let y4 = float_of_int yy1 -. ything;
    let vertexBuffer =
      makeRectVertexBuffer (x2, y2) (x3, y3) (x1, y1) (x4, y4) (!env).stroke.color;
    drawVertexBuffer ::vertexBuffer mode::Constants.triangle_strip !env
  };
  let ellipse env a b c d => drawEllipseInternal !env a b c d;
};

type userCallbackT 'a = 'a => ref glState => ('a, glState);

let afterDraw f (env: ref glEnv) => {
  let rate = int_of_float (1000. /. f);
  env := {
    ...!env,
    mouse: {...(!env).mouse, prevPos: (!env).mouse.pos},
    frame: {count: (!env).frame.count + 1, rate}
  }
};

module ReProcessor: ReProcessorT = {
  type t = ref glEnv;
  let run ::setup ::draw=? ::mouseMove=? ::mouseDragged=? ::mouseDown=? ::mouseUp=? () => {
    Random.self_init ();
    PUtils.noiseSeed (Random.int (PUtils.pow 2 30 - 1));
    let env = ref (createCanvas (Gl.Window.init argv::Sys.argv) 200 200);
    let userState = ref (setup env);
    Gl.render
      window::(!env).window
      displayFunc::(
        switch draw {
        | Some draw => (
            fun f => {
              userState := draw !userState env;
              afterDraw f env
            }
          )
        | None => (fun f => ())
        }
      )
      mouseDown::(
        fun ::button ::state ::x ::y => {
          env := {...!env, mouse: {...(!env).mouse, pos: (x, y), pressed: true}};
          switch mouseDown {
          | Some mouseDown => userState := mouseDown !userState env
          | None => ()
          }
        }
      )
      mouseUp::(
        fun ::button ::state ::x ::y => {
          env := {...!env, mouse: {...(!env).mouse, pos: (x, y), pressed: false}};
          switch mouseUp {
          | Some mouseUp => userState := mouseUp !userState env
          | None => ()
          }
        }
      )
      mouseMove::(
        fun ::x ::y => {
          env := {...!env, mouse: {...(!env).mouse, pos: (x, y)}};
          if (!env).mouse.pressed {
            switch mouseDragged {
            | Some mouseDragged => userState := mouseDragged !userState env
            | None => ()
            }
          } else {
            switch mouseMove {
            | Some mouseMove => userState := mouseMove !userState env
            | None => ()
            }
          }
        }
      )
      windowResize::(
        fun () =>
          if (!env).size.resizeable {
            let height = Gl.Window.getHeight (!env).window;
            let width = Gl.Window.getWidth (!env).window;
            resetSize env width height
          } else {
            P.size env (P.width env) (P.height env)
          }
      )
      ()
  };
};
