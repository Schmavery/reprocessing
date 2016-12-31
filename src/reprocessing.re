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

type glEnv = {
  camera: glCamera,
  window: Gl.Window.t,
  gl: Gl.contextT,
  vertexBuffer: Gl.bufferT,
  colorBuffer: Gl.bufferT,
  aVertexColor: Gl.attributeT,
  aVertexPosition: Gl.attributeT,
  pMatrixUniform: Gl.uniformT,
  currFill: color,
  currBackground: color,
  mouse: mouseT,
  stroke: strokeT
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

  uniform mat4 uPMatrix;

  varying vec4 vColor;

  void main(void) {
    gl_Position = uPMatrix * vec4(aVertexPosition, 1.0);
    vColor = aVertexColor;
  }
|};

let fragmentShaderSource = {|
  varying vec4 vColor;

  void main(void) {
    gl_FragColor = vColor;
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
  let colorBuffer = Gl.createBuffer gl;
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

  /**
   * Will mutate the projectionMatrix to be an ortho matrix with the given boundaries.
   * See this link for quick explanation of what this is.
   * https://shearer12345.github.io/graphics/assets/projectionPerspectiveVSOrthographic.png
   */
  Gl.Mat4.ortho
    out::camera.projectionMatrix
    left::0.
    right::(float_of_int (Gl.Window.getWidth window))
    bottom::(float_of_int (Gl.Window.getHeight window))
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
    colorBuffer,
    aVertexPosition,
    aVertexColor,
    pMatrixUniform,
    currFill,
    currBackground,
    mouse: {pos: (0, 0), prevPos: (0, 0), pressed: false},
    stroke: {color: {r: 0, g: 0, b: 0}, weight: 10}
  }
};

module PUtils = {
  let color ::r ::g ::b :color => {r, g, b};
  let round i => floor (i +. 0.5);
};

let drawRectInternal (x1, y1) (x2, y2) (x3, y3) (x4, y4) color env => {

  /** Setup vertices to be sent to the GPU **/
  let square_vertices = [|x1, y1, 0.0, x2, y2, 0.0, x3, y3, 0.0, x4, y4, 0.0|];
  Gl.bindBuffer context::env.gl target::Constants.array_buffer buffer::env.vertexBuffer;
  Gl.bufferData
    context::env.gl
    target::Constants.array_buffer
    data::(Gl.Float32 square_vertices)
    usage::Constants.static_draw;
  Gl.vertexAttribPointer
    context::env.gl
    attribute::env.aVertexPosition
    size::3
    type_::Constants.float_
    normalize::false
    stride::0
    offset::0;

  /** Setup colors to be sent to the GPU **/
  let toColorFloat i => float_of_int i /. 255.;
  let (r, g, b) = (
    toColorFloat color.r,
    toColorFloat color.g,
    toColorFloat color.b
  );
  let square_colors = ref [];
  for i in 0 to 3 {
    square_colors := [r, g, b, 1., ...!square_colors]
  };
  Gl.bindBuffer context::env.gl target::Constants.array_buffer buffer::env.colorBuffer;
  Gl.bufferData
    context::env.gl
    target::Constants.array_buffer
    data::(Gl.Float32 (Array.of_list !square_colors))
    usage::Constants.static_draw;
  Gl.vertexAttribPointer
    context::env.gl
    attribute::env.aVertexColor
    size::4
    type_::Constants.float_
    normalize::false
    stride::0
    offset::0;
  Gl.uniformMatrix4fv
    context::env.gl location::env.pMatrixUniform value::env.camera.projectionMatrix;

  /** Final call which actually does the "draw" **/
  Gl.drawArrays context::env.gl mode::Constants.triangle_strip first::0 count::4
};

module P = {
  let width env => Gl.Window.getWidth (!env).window;
  let height env => Gl.Window.getHeight (!env).window;
  let mouse env => (!env).mouse.pos;
  let pmouse env => (!env).mouse.prevPos;
  let mousePressed env => (!env).mouse.pressed;
  let background (env: ref glEnv) (c: color) => env := {...!env, currBackground: c};
  let fill (env: ref glEnv) (c: color) => env := {...!env, currFill: c};
  let size (env: ref glEnv) width height => {
    Gl.Window.setWindowSize window::(!env).window ::width ::height;
    Gl.viewport context::(!env).gl x::0 y::0 ::width ::height;
    Gl.clearColor context::(!env).gl r::0. g::0. b::0. a::1.;
    Gl.Mat4.ortho
      out::(!env).camera.projectionMatrix
      left::0.
      right::(float_of_int width)
      bottom::(float_of_int height)
      top::0.
      near::0.
      far::100.
  };
  let rect (env: ref glEnv) x y width height =>
    drawRectInternal
      (float_of_int @@ x + width, float_of_int @@ y + height)
      (float_of_int x, float_of_int @@ y + height)
      (float_of_int @@ x + width, float_of_int y)
      (float_of_int x, float_of_int y)
      (!env).currFill
      !env;
  let background env color => {
    let width = Gl.Window.getWidth (!env).window;
    let height = Gl.Window.getHeight (!env).window;
    let oldEnv = !env;
    fill env color;
    rect env 0 0 width height;
    env := oldEnv
  };
  let clear env => Gl.clear (!env).gl (Constants.color_buffer_bit lor Constants.depth_buffer_bit);
  let stroke env color => env := {...!env, stroke: {...(!env).stroke, color}};
  let lineWeight env weight => env := {...!env, stroke: {...(!env).stroke, weight}};
  let line env (xx1, yy1) (xx2, yy2) => {
    let dx = xx2 - xx1;
    let dy = yy2 - yy1;
    let mag = sqrt (float_of_int (dx * dx + dy * dy));
    let radius = float_of_int (!env).stroke.weight /. 2.;
    let xthing = PUtils.round (float_of_int (yy2 - yy1) /. mag *. radius);
    let ything = PUtils.round (-. float_of_int (xx2 - xx1) /. mag *. radius);
    let x1 = float_of_int xx1 +. xthing;
    let y1 = float_of_int yy1 +. ything;
    let x2 = float_of_int xx2 +. xthing;
    let y2 = float_of_int yy2 +. ything;
    let x3 = float_of_int xx2 -. xthing;
    let y3 = float_of_int yy2 -. ything;
    let x4 = float_of_int xx1 -. xthing;
    let y4 = float_of_int yy1 -. ything;
    drawRectInternal (x2, y2) (x3, y3) (x1, y1) (x4, y4) (!env).stroke.color !env
    /* drawRectInternal  */
  };
};

type userCallbackT 'a = 'a => ref glState => ('a, glState);

let afterDraw (env: ref glEnv) =>
  env := {...!env, mouse: {...(!env).mouse, prevPos: (!env).mouse.pos}};

module ReProcessor: ReProcessorT = {
  type t = ref glEnv;
  let run ::setup ::draw=? ::mouseMove=? ::mouseDragged=? ::mouseDown=? ::mouseUp=? () => {
    let env = ref (createCanvas (Gl.Window.init argv::Sys.argv) 200 200);
    let userState = ref (setup env);
    Gl.render
      window::(!env).window
      displayFunc::(
        switch draw {
        | Some draw => (
            fun f => {
              userState := draw !userState env;
              afterDraw env
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
      ()
  };
};
