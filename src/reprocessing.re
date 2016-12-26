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
  mouseX: int,
  mouseY: int,
  pmouseX: int,
  pmouseY: int,
  mousePressed: bool
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
    mouseX: 0,
    mouseY: 0,
    pmouseX: 0,
    pmouseY: 0,
    mousePressed: false
  }
};

module PUtils = {
  let color ::r ::g ::b :color => {r, g, b};
};

module P = {
  let mouseX env => (!env).mouseX;
  let mouseY env => (!env).mouseY;
  let pmouseX env => (!env).pmouseX;
  let pmouseY env => (!env).pmouseY;
  let mousePressed env => (!env).mousePressed;
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
  let rect (env: ref glEnv) x y width height => {
    let env = !env;

    /** Setup vertices to be sent to the GPU **/
    let square_vertices = [|
      float_of_int @@ x + width,
      float_of_int @@ y + height,
      0.0,
      float_of_int x,
      float_of_int @@ y + height,
      0.0,
      float_of_int @@ x + width,
      float_of_int y,
      0.0,
      float_of_int x,
      float_of_int y,
      0.0
    |];
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
      toColorFloat env.currFill.r,
      toColorFloat env.currFill.g,
      toColorFloat env.currFill.b
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
  let clear env => Gl.clear (!env).gl (Constants.color_buffer_bit lor Constants.depth_buffer_bit);
};

type userCallbackT 'a = 'a => ref glState => ('a, glState);

module type ReProcessorT = {
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

module ReProcessor: ReProcessorT = {
  type t;
  let run ::setup ::draw=? ::mouseMove=? ::mouseDragged=? ::mouseDown=? ::mouseUp=? () => {
    let env = ref (createCanvas (Gl.Window.init argv::Sys.argv) 200 200);
    let userState = ref (setup env);
    Gl.render
      window::(!env).window
      displayFunc::(
        switch draw {
        | Some draw => (fun f => userState := draw !userState env)
        | None => (fun f => ())
        }
      )
      mouseDown::(
        fun ::button ::state ::x ::y => {
          env := {
            ...!env,
            pmouseX: (!env).mouseX,
            pmouseY: (!env).mouseY,
            mouseX: x,
            mouseY: y,
            mousePressed: true
          };
          switch mouseDown {
          | Some mouseDown => userState := mouseDown !userState env
          | None => ()
          }
        }
      )
      mouseUp::(
        fun ::button ::state ::x ::y => {
          env := {
            ...!env,
            pmouseX: (!env).mouseX,
            pmouseY: (!env).mouseY,
            mouseX: x,
            mouseY: y,
            mousePressed: false
          };
          switch mouseUp {
          | Some mouseUp => userState := mouseUp !userState env
          | None => ()
          }
        }
      )
      mouseMove::(
        fun ::x ::y => {
          env := {...!env, pmouseX: (!env).mouseX, pmouseY: (!env).mouseY, mouseX: x, mouseY: y};
          if (!env).mousePressed {
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
