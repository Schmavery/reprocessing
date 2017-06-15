open Common;

open Glloader;

open Utils;

let getProgram
    ::context
    vertexShader::(vertexShaderSource: string)
    fragmentShader::(fragmentShaderSource: string)
    :option Gl.programT => {
  let vertexShader = Gl.createShader ::context Constants.vertex_shader;
  Gl.shaderSource ::context shader::vertexShader source::vertexShaderSource;
  Gl.compileShader ::context vertexShader;
  let compiledCorrectly =
    Gl.getShaderParameter ::context shader::vertexShader paramName::Gl.Compile_status == 1;
  if compiledCorrectly {
    let fragmentShader = Gl.createShader ::context Constants.fragment_shader;
    Gl.shaderSource ::context shader::fragmentShader source::fragmentShaderSource;
    Gl.compileShader ::context fragmentShader;
    let compiledCorrectly =
      Gl.getShaderParameter ::context shader::fragmentShader paramName::Gl.Compile_status == 1;
    if compiledCorrectly {
      let program = Gl.createProgram ::context;
      Gl.attachShader ::context ::program shader::vertexShader;
      Gl.deleteShader ::context vertexShader;
      Gl.attachShader ::context ::program shader::fragmentShader;
      Gl.deleteShader ::context fragmentShader;
      Gl.linkProgram ::context program;
      let linkedCorrectly =
        Gl.getProgramParameter ::context ::program paramName::Gl.Link_status == 1;
      if linkedCorrectly {
        Some program
      } else {
        print_endline @@ "Linking error: " ^ Gl.getProgramInfoLog ::context program;
        None
      }
    } else {
      print_endline @@ "Fragment shader error: " ^ Gl.getShaderInfoLog ::context fragmentShader;
      None
    }
  } else {
    print_endline @@ "Vertex shader error: " ^ Gl.getShaderInfoLog ::context vertexShader;
    None
  }
};

let createCanvas window (height: int) (width: int) :glEnv => {
  Gl.Window.setWindowSize ::window ::width ::height;
  let context = Gl.Window.getContext window;
  Gl.viewport ::context x::(-1) y::(-1) ::width ::height;
  Gl.clearColor ::context r::0. g::0. b::0. a::1.;
  Gl.clear ::context mask::(Constants.color_buffer_bit lor Constants.depth_buffer_bit);

  /** Camera is a simple record containing one matrix used to project a point in 3D onto the screen. **/
  let camera = {projectionMatrix: Gl.Mat4.create ()};
  let vertexBuffer = Gl.createBuffer ::context;
  let elementBuffer = Gl.createBuffer ::context;
  let program =
    switch (
      getProgram
        ::context
        vertexShader::Shaders.vertexShaderSource
        fragmentShader::Shaders.fragmentShaderSource
    ) {
    | None => failwith "Could not create the program and/or the shaders. Aborting."
    | Some program => program
    };
  Gl.useProgram ::context program;

  /** Get the attribs ahead of time to be used inside the render function **/
  let aVertexPosition = Gl.getAttribLocation ::context ::program name::"aVertexPosition";
  Gl.enableVertexAttribArray ::context attribute::aVertexPosition;
  let aVertexColor = Gl.getAttribLocation ::context ::program name::"aVertexColor";
  Gl.enableVertexAttribArray ::context attribute::aVertexColor;
  let pMatrixUniform = Gl.getUniformLocation ::context ::program name::"uPMatrix";
  Gl.uniformMatrix4fv ::context location::pMatrixUniform value::camera.projectionMatrix;

  /** Get attribute and uniform locations for later usage in the draw code. **/
  let aTextureCoord = Gl.getAttribLocation ::context ::program name::"aTextureCoord";
  Gl.enableVertexAttribArray ::context attribute::aTextureCoord;

  /** Generate texture buffer that we'll use to pass image data around. **/
  let texture = Gl.createTexture ::context;

  /** This tells OpenGL that we're going to be using texture0. OpenGL imposes a limit on the number of
      texture we can manipulate at the same time. That limit depends on the device. We don't care as we'll just
      always use texture0. **/
  Gl.activeTexture ::context Constants.texture0;

  /** Bind `texture` to `texture_2d` to modify it's magnification and minification params. **/
  Gl.bindTexture ::context target::Constants.texture_2d ::texture;
  let uSampler = Gl.getUniformLocation ::context ::program name::"uSampler";

  /** Load a dummy texture. This is because we're using the same shader for things with and without a texture */
  Gl.texImage2D_RGBA
    ::context
    target::Constants.texture_2d
    level::0
    width::1
    height::1
    border::0
    data::(Gl.Bigarray.of_array Gl.Bigarray.Uint8 [|0, 0, 0, 0|]);
  Gl.texParameteri
    ::context
    target::Constants.texture_2d
    pname::Constants.texture_mag_filter
    param::Constants.linear;
  Gl.texParameteri
    ::context
    target::Constants.texture_2d
    pname::Constants.texture_min_filter
    param::Constants.linear_mipmap_nearest;

  /** Enable blend and tell OpenGL how to blend. */
  Gl.enable ::context Constants.blend;
  Gl.blendFunc ::context Constants.src_alpha Constants.one_minus_src_alpha;

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
    far::1.;
  {
    camera,
    window,
    gl: context,
    batch: {
      vertexArray: Gl.Bigarray.create Gl.Bigarray.Float32 (circularBufferSize * vertexSize),
      elementArray: Gl.Bigarray.create Gl.Bigarray.Uint16 circularBufferSize,
      vertexPtr: 0,
      elementPtr: 0,
      currTex: None,
      nullTex: texture
    },
    vertexBuffer,
    elementBuffer,
    aVertexPosition,
    aTextureCoord,
    aVertexColor,
    pMatrixUniform,
    uSampler,
    keyboard: {keyCode: Gl.Events.Nothing},
    mouse: {pos: (0, 0), prevPos: (0, 0), pressed: false},
    style: {
      fillColor: Some {r: 0, g: 0, b: 0},
      strokeWeight: 10,
      strokeColor: Some {r: 0, g: 0, b: 0}
    },
    styleStack: [],
    matrix: Matrix.createIdentity (),
    matrixStack: [],
    frame: {count: 1, rate: 10},
    size: {height, width, resizeable: true}
  }
};

let drawGeometry
    vertexArray::(vertexArray: Gl.Bigarray.t float Gl.Bigarray.float32_elt)
    elementArray::(elementArray: Gl.Bigarray.t int Gl.Bigarray.int16_unsigned_elt)
    ::mode
    ::count
    ::textureBuffer
    env => {
  /* Bind `vertexBuffer`, a pointer to chunk of memory to be sent to the GPU to the "register" called
     `array_buffer` */
  Gl.bindBuffer context::env.gl target::Constants.array_buffer buffer::env.vertexBuffer;

  /** Copy all of the data over into whatever's in `array_buffer` (so here it's `vertexBuffer`) **/
  Gl.bufferData
    context::env.gl target::Constants.array_buffer data::vertexArray usage::Constants.stream_draw;

  /** Tell the GPU about the shader attribute called `aVertexPosition` so it can access the data per vertex */
  Gl.vertexAttribPointer
    context::env.gl
    attribute::env.aVertexPosition
    size::2
    type_::Constants.float_
    normalized::false
    stride::(vertexSize * 4)
    offset::0;

  /** Same as above but for `aVertexColor` **/
  Gl.vertexAttribPointer
    context::env.gl
    attribute::env.aVertexColor
    size::4
    type_::Constants.float_
    normalized::false
    stride::(vertexSize * 4)
    offset::(2 * 4);

  /** Same as above but for `aTextureCoord` **/
  Gl.vertexAttribPointer
    context::env.gl
    attribute::env.aTextureCoord
    size::2
    type_::Constants.float_
    normalized::false
    stride::(vertexSize * 4)
    offset::(6 * 4);

  /** Tell OpenGL about what the uniform called `uSampler` is pointing at, here it's given 0 which is what
      texture0 represent.  **/
  Gl.uniform1i context::env.gl location::env.uSampler val::0;

  /** Bind `elementBuffer`, a pointer to GPU memory to `element_array_buffer`. That "register" is used for
      the data representing the indices of the vertex. **/
  Gl.bindBuffer context::env.gl target::Constants.element_array_buffer buffer::env.elementBuffer;

  /** Copy the `elementArray` into whatever buffer is in `element_array_buffer` **/
  Gl.bufferData
    context::env.gl
    target::Constants.element_array_buffer
    data::elementArray
    usage::Constants.stream_draw;

  /** We bind `texture` to texture_2d, like we did for the vertex buffers in some ways (I think?) **/
  Gl.bindTexture context::env.gl target::Constants.texture_2d texture::textureBuffer;

  /** Final call which actually tells the GPU to draw. **/
  Gl.drawElements context::env.gl ::mode ::count type_::Constants.unsigned_short offset::0
};

/*
 * Helper that will send the currently available data inside globalVertexArray.
 * This function assumes that the vertex data is stored as simple triangles.
 *
 * That function creates a new big array with a new size given the offset and len but does NOT copy the
 * underlying array of memory. So mutation done to that sub array will be reflected in the original one.
 */
let flushGlobalBatch env =>
  if (env.batch.elementPtr > 0) {
    let textureBuffer =
      switch env.batch.currTex {
      | None => env.batch.nullTex
      | Some textureBuffer => textureBuffer
      };
    drawGeometry
      vertexArray::(Gl.Bigarray.sub env.batch.vertexArray offset::0 len::env.batch.vertexPtr)
      elementArray::(Gl.Bigarray.sub env.batch.elementArray offset::0 len::env.batch.elementPtr)
      mode::Constants.triangles
      count::env.batch.elementPtr
      ::textureBuffer
      env;
    env.batch.currTex = None;
    env.batch.vertexPtr = 0;
    env.batch.elementPtr = 0
  };

let maybeFlushBatch env texture adding =>
  if (
    env.batch.elementPtr + adding >= circularBufferSize ||
    env.batch.elementPtr > 0 && env.batch.currTex !== texture
  ) {
    flushGlobalBatch env
  };

let toColorFloat i => float_of_int i /. 255.;

/*
 * This array packs all of the values that the shaders need: vertices, colors and texture coordinates.
 * We put them all in one as an optimization, so there are less back and forths between us and the GPU.
 *
 * The vertex array looks like:
 *
 * |<--------  8 * 4 bytes  ------->|
 *  --------------------------------
 * |  x  y  |  r  g  b  a  |  s  t  |  x2  y2  |  r2  g2  b2  a2  |  s2  t2  | ....
 *  --------------------------------
 * |           |              |
 * +- offset: 0 bytes, stride: 8 * 4 bytes (because we need to move by 8*4 bytes to get to the next x)
 *             |              |
 *             +- offset: 3 * 4 bytes, stride: 8 * 4 bytes
 *                            |
 *                            +- offset: (3 + 4) * 4 bytes, stride: 8 * 4 bytes
 *
 *
 * The element array is just an array of indices of vertices given that each vertex takes 8 * 4 bytes.
 * For example, if the element array looks like [|0, 1, 2, 1, 2, 3|], we're telling the GPU to draw 2
 * triangles: one with the vertices 0, 1 and 2 from the vertex array, and one with the vertices 1, 2 and 3.
 * We can "point" to duplicated vertices in our geometry to avoid sending those vertices.
 */
let addRectToGlobalBatch
    env
    bottomRight::(x1, y1)
    bottomLeft::(x2, y2)
    topRight::(x3, y3)
    topLeft::(x4, y4)
    color::{r, g, b} => {
  maybeFlushBatch env None 6;
  let set = Gl.Bigarray.set;
  let (r, g, b) = (toColorFloat r, toColorFloat g, toColorFloat b);
  let i = env.batch.vertexPtr;
  let vertexArrayToMutate = env.batch.vertexArray;
  set vertexArrayToMutate (i + 0) x1;
  set vertexArrayToMutate (i + 1) y1;
  set vertexArrayToMutate (i + 2) r;
  set vertexArrayToMutate (i + 3) g;
  set vertexArrayToMutate (i + 4) b;
  set vertexArrayToMutate (i + 5) 1.;
  set vertexArrayToMutate (i + 6) 0.0;
  set vertexArrayToMutate (i + 7) 0.0;
  set vertexArrayToMutate (i + 8) x2;
  set vertexArrayToMutate (i + 9) y2;
  set vertexArrayToMutate (i + 10) r;
  set vertexArrayToMutate (i + 11) g;
  set vertexArrayToMutate (i + 12) b;
  set vertexArrayToMutate (i + 13) 1.;
  set vertexArrayToMutate (i + 14) 0.0;
  set vertexArrayToMutate (i + 15) 0.0;
  set vertexArrayToMutate (i + 16) x3;
  set vertexArrayToMutate (i + 17) y3;
  set vertexArrayToMutate (i + 18) r;
  set vertexArrayToMutate (i + 19) g;
  set vertexArrayToMutate (i + 20) b;
  set vertexArrayToMutate (i + 21) 1.;
  set vertexArrayToMutate (i + 22) 0.0;
  set vertexArrayToMutate (i + 23) 0.0;
  set vertexArrayToMutate (i + 24) x4;
  set vertexArrayToMutate (i + 25) y4;
  set vertexArrayToMutate (i + 26) r;
  set vertexArrayToMutate (i + 27) g;
  set vertexArrayToMutate (i + 28) b;
  set vertexArrayToMutate (i + 29) 1.;
  set vertexArrayToMutate (i + 30) 0.0;
  set vertexArrayToMutate (i + 31) 0.0;
  let ii = i / vertexSize;
  let j = env.batch.elementPtr;
  let elementArrayToMutate = env.batch.elementArray;
  set elementArrayToMutate (j + 0) ii;
  set elementArrayToMutate (j + 1) (ii + 1);
  set elementArrayToMutate (j + 2) (ii + 2);
  set elementArrayToMutate (j + 3) (ii + 1);
  set elementArrayToMutate (j + 4) (ii + 2);
  set elementArrayToMutate (j + 5) (ii + 3);
  env.batch.vertexPtr = i + 4 * vertexSize;
  env.batch.elementPtr = j + 6
};

let drawTriangleInternal env (x1, y1) (x2, y2) (x3, y3) color::{r, g, b} => {
  maybeFlushBatch env None 3;
  let set = Gl.Bigarray.set;
  let (r, g, b) = (toColorFloat r, toColorFloat g, toColorFloat b);
  let i = env.batch.vertexPtr;
  let vertexArrayToMutate = env.batch.vertexArray;
  set vertexArrayToMutate (i + 0) x1;
  set vertexArrayToMutate (i + 1) y1;
  set vertexArrayToMutate (i + 2) r;
  set vertexArrayToMutate (i + 3) g;
  set vertexArrayToMutate (i + 4) b;
  set vertexArrayToMutate (i + 5) 1.;
  set vertexArrayToMutate (i + 6) 0.0;
  set vertexArrayToMutate (i + 7) 0.0;
  set vertexArrayToMutate (i + 8) x2;
  set vertexArrayToMutate (i + 9) y2;
  set vertexArrayToMutate (i + 10) r;
  set vertexArrayToMutate (i + 11) g;
  set vertexArrayToMutate (i + 12) b;
  set vertexArrayToMutate (i + 13) 1.;
  set vertexArrayToMutate (i + 14) 0.0;
  set vertexArrayToMutate (i + 15) 0.0;
  set vertexArrayToMutate (i + 16) x3;
  set vertexArrayToMutate (i + 17) y3;
  set vertexArrayToMutate (i + 18) r;
  set vertexArrayToMutate (i + 19) g;
  set vertexArrayToMutate (i + 20) b;
  set vertexArrayToMutate (i + 21) 1.;
  set vertexArrayToMutate (i + 22) 0.0;
  set vertexArrayToMutate (i + 23) 0.0;
  let ii = i / vertexSize;
  let j = env.batch.elementPtr;
  let elementArrayToMutate = env.batch.elementArray;
  set elementArrayToMutate (j + 0) ii;
  set elementArrayToMutate (j + 1) (ii + 1);
  set elementArrayToMutate (j + 2) (ii + 2);
  env.batch.vertexPtr = i + 3 * vertexSize;
  env.batch.elementPtr = j + 3
};

let drawLineInternal env (xx1, yy1) (xx2, yy2) color => {
  let dx = xx2 -. xx1;
  let dy = yy2 -. yy1;
  let mag = PUtils.distf (xx1, yy1) (xx2, yy2);
  let radius = float_of_int env.style.strokeWeight /. 2.;
  let xthing = dy /. mag *. radius;
  let ything = -. dx /. mag *. radius;
  let x1 = xx2 +. xthing;
  let y1 = yy2 +. ything;
  let x2 = xx1 +. xthing;
  let y2 = yy1 +. ything;
  let x3 = xx2 -. xthing;
  let y3 = yy2 -. ything;
  let x4 = xx1 -. xthing;
  let y4 = yy1 -. ything;
  addRectToGlobalBatch env (x1, y1) (x2, y2) (x3, y3) (x4, y4) color
};

let drawArcInternal
    env
    (xCenterOfCircle: float, yCenterOfCircle: float)
    (radx: float)
    (rady: float)
    (start: float)
    (stop: float)
    (isPie: bool)
    (matrix: array float)
    {r, g, b} => {
  let transform = Matrix.matptmul matrix;
  let noOfFans = int_of_float (radx +. rady) * 2 + 10;
  maybeFlushBatch env None ((noOfFans - 3) * 3 + 3);
  let pi = 4.0 *. atan 1.0;
  let anglePerFan = 2. *. pi /. float_of_int noOfFans;
  let (r, g, b) = (toColorFloat r, toColorFloat g, toColorFloat b);
  let verticesData = env.batch.vertexArray;
  let elementData = env.batch.elementArray;
  let set = Gl.Bigarray.set;
  let get = Gl.Bigarray.get;
  let vertexArrayOffset = env.batch.vertexPtr;
  let elementArrayOffset = env.batch.elementPtr;
  let start_i =
    if isPie {
      /* Start one earlier and force the first point to be the center */
      int_of_float (start /. anglePerFan) - 2
    } else {
      int_of_float (start /. anglePerFan) - 1
    };
  let stop_i = int_of_float (stop /. anglePerFan) - 1;
  for i in start_i to stop_i {
    let (xCoordinate, yCoordinate) =
      transform (
        if (isPie && i - start_i == 0) {
          (
            /* force the first point to be the center */
            xCenterOfCircle,
            yCenterOfCircle
          )
        } else {
          let angle = anglePerFan *. float_of_int (i + 1);
          (xCenterOfCircle +. cos angle *. radx, yCenterOfCircle +. sin angle *. rady)
        }
      );
    let ii = (i - start_i) * vertexSize + vertexArrayOffset;
    set verticesData (ii + 0) xCoordinate;
    set verticesData (ii + 1) yCoordinate;
    set verticesData (ii + 2) r;
    set verticesData (ii + 3) g;
    set verticesData (ii + 4) b;
    set verticesData (ii + 5) 1.0;
    set verticesData (ii + 6) 0.0;
    set verticesData (ii + 7) 0.0;
    /* For the first three vertices, we don't do any deduping. Then for the subsequent ones, we'll actually
       have 3 elements, one pointing at the first vertex, one pointing at the previously added vertex and one
       pointing at the current vertex. This mimicks the behavior of triangle_fan. */
    if (i - start_i < 3) {
      set elementData (i - start_i + elementArrayOffset) (ii / vertexSize)
    } else {
      /* We've already added 3 elements, for i = 0, 1 and 2. From now on, we'll add 3 elements _per_ i.
         To calculate the correct offset in `elementData` we remove 3 from i as if we're starting from 0 (the
         first time we enter this loop i = 3), then for each i we'll add 3 elements (so multiply by 3) BUT for
         i = 3 we want `jj` to be 3 so we shift everything by 3 (so add 3). Everything's also shifted by
         `elementArrayOffset` */
      let jj = (i - start_i - 3) * 3 + elementArrayOffset + 3;
      set elementData jj (vertexArrayOffset / vertexSize);
      set elementData (jj + 1) (get elementData (jj - 1));
      set elementData (jj + 2) (ii / vertexSize)
    }
  };
  env.batch.vertexPtr = env.batch.vertexPtr + noOfFans * vertexSize;
  env.batch.elementPtr = env.batch.elementPtr + (stop_i - start_i - 3) * 3 + 3
};

let drawEllipseInternal env center (radx: float) (rady: float) (matrix: array float) c =>
  drawArcInternal env center radx rady 0. PConstants.tau false matrix c;

let drawArcStroke
    env
    (xCenterOfCircle: float, yCenterOfCircle: float)
    (radx: float)
    (rady: float)
    (start: float)
    (stop: float)
    (isOpen: bool)
    (isPie: bool)
    (matrix: array float)
    ({r, g, b} as strokeColor)
    strokeWidth => {
  let transform = Matrix.matptmul matrix;
  let (r, g, b) = (toColorFloat r, toColorFloat g, toColorFloat b);
  let verticesData = env.batch.vertexArray;
  let elementData = env.batch.elementArray;
  let set = Gl.Bigarray.set;
  let noOfFans = int_of_float (radx +. rady) * 2 + 10;
  maybeFlushBatch env None ((noOfFans - 3) * 3 + 3);
  let pi = 4.0 *. atan 1.0;
  let anglePerFan = 2. *. pi /. float_of_int noOfFans;
  /* I calculated this roughly by doing:
     anglePerFan *. float_of_int (i + 1) == start
     i+1 == start /. anglePerFan
     */
  let start_i = int_of_float (start /. anglePerFan) - 1;
  let stop_i = int_of_float (stop /. anglePerFan) - 1;
  let prevEl: ref (option (int, int)) = ref None;
  let halfwidth = float_of_int strokeWidth /. 2.;
  for i in start_i to stop_i {
    let angle = anglePerFan *. float_of_int (i + 1);
    let (xCoordinateInner, yCoordinateInner) =
      transform (
        xCenterOfCircle +. cos angle *. (radx -. halfwidth),
        yCenterOfCircle +. sin angle *. (rady -. halfwidth)
      );
    let (xCoordinateOuter, yCoordinateOuter) =
      transform (
        xCenterOfCircle +. cos angle *. (radx +. halfwidth),
        yCenterOfCircle +. sin angle *. (rady +. halfwidth)
      );
    let ii = env.batch.vertexPtr;
    set verticesData (ii + 0) xCoordinateInner;
    set verticesData (ii + 1) yCoordinateInner;
    set verticesData (ii + 2) r;
    set verticesData (ii + 3) g;
    set verticesData (ii + 4) b;
    set verticesData (ii + 5) 1.0;
    set verticesData (ii + 6) 0.0;
    set verticesData (ii + 7) 0.0;
    let ii = ii + vertexSize;
    set verticesData (ii + 0) xCoordinateOuter;
    set verticesData (ii + 1) yCoordinateOuter;
    set verticesData (ii + 2) r;
    set verticesData (ii + 3) g;
    set verticesData (ii + 4) b;
    set verticesData (ii + 5) 1.0;
    set verticesData (ii + 6) 0.0;
    set verticesData (ii + 7) 0.0;
    env.batch.vertexPtr = env.batch.vertexPtr + vertexSize * 2;
    let currOuter = ii / vertexSize;
    let currInner = ii / vertexSize - 1;
    let currEl = Some (currInner, currOuter);
    switch !prevEl {
    | None => prevEl := currEl
    | Some (prevInner, prevOuter) =>
      let elementArrayOffset = env.batch.elementPtr;
      set elementData elementArrayOffset prevInner;
      set elementData (elementArrayOffset + 1) prevOuter;
      set elementData (elementArrayOffset + 2) currOuter;
      set elementData (elementArrayOffset + 3) currOuter;
      set elementData (elementArrayOffset + 4) prevInner;
      set elementData (elementArrayOffset + 5) currInner;
      env.batch.elementPtr = env.batch.elementPtr + 6;
      prevEl := currEl
    }
  };
  if (not isOpen) {
    let (startX, startY) =
      transform (xCenterOfCircle +. cos start *. radx, yCenterOfCircle +. sin start *. rady);
    let (stopX, stopY) =
      transform (xCenterOfCircle +. cos stop *. radx, yCenterOfCircle +. sin stop *. rady);
    if isPie {
      drawLineInternal env (startX, startY) (xCenterOfCircle, yCenterOfCircle) strokeColor;
      drawLineInternal env (stopX, stopY) (xCenterOfCircle, yCenterOfCircle) strokeColor;
      drawEllipseInternal
        env (transform (xCenterOfCircle, yCenterOfCircle)) halfwidth halfwidth matrix strokeColor
    } else {
      drawLineInternal env (startX, startY) (stopX, stopY) strokeColor
    };
    drawEllipseInternal env (startX, startY) halfwidth halfwidth matrix strokeColor;
    drawEllipseInternal env (stopX, stopY) halfwidth halfwidth matrix strokeColor
  }
};

let loadImage (env: glEnv) filename :imageT => {
  let imageRef = ref None;
  Gl.loadImage
    ::filename
    callback::(
      fun imageData =>
        switch imageData {
        | None => failwith ("Could not load image '" ^ filename ^ "'.") /* TODO: handle this better? */
        | Some img =>
          let env = env;
          let textureBuffer = Gl.createTexture context::env.gl;
          let height = Gl.getImageHeight img;
          let width = Gl.getImageWidth img;
          imageRef := Some {img, textureBuffer, height, width};
          Gl.bindTexture context::env.gl target::Constants.texture_2d texture::textureBuffer;
          Gl.texImage2DWithImage context::env.gl target::Constants.texture_2d level::0 image::img;
          Gl.texParameteri
            context::env.gl
            target::Constants.texture_2d
            pname::Constants.texture_mag_filter
            param::Constants.linear;
          Gl.texParameteri
            context::env.gl
            target::Constants.texture_2d
            pname::Constants.texture_min_filter
            param::Constants.linear
        }
    )
    ();
  imageRef
};

let drawImageInternal {width, height, textureBuffer} ::x ::y ::subx ::suby ::subw ::subh env => {
  maybeFlushBatch env (Some textureBuffer) 6;
  let (fsubx, fsuby, fsubw, fsubh) = (
    float_of_int subx /. float_of_int width,
    float_of_int suby /. float_of_int height,
    float_of_int subw /. float_of_int width,
    float_of_int subh /. float_of_int height
  );
  let (x1, y1) = (float_of_int @@ x + subw, float_of_int @@ y + subh);
  let (x2, y2) = (float_of_int x, float_of_int @@ y + subh);
  let (x3, y3) = (float_of_int @@ x + subw, float_of_int y);
  let (x4, y4) = (float_of_int x, float_of_int y);
  let set = Gl.Bigarray.set;
  let ii = env.batch.vertexPtr;
  let vertexArray = env.batch.vertexArray;
  set vertexArray (ii + 0) x1;
  set vertexArray (ii + 1) y1;
  set vertexArray (ii + 2) 0.0;
  set vertexArray (ii + 3) 0.0;
  set vertexArray (ii + 4) 0.0;
  set vertexArray (ii + 5) 0.0;
  set vertexArray (ii + 6) (fsubx +. fsubw);
  set vertexArray (ii + 7) (fsuby +. fsubh);
  set vertexArray (ii + 8) x2;
  set vertexArray (ii + 9) y2;
  set vertexArray (ii + 10) 0.0;
  set vertexArray (ii + 11) 0.0;
  set vertexArray (ii + 12) 0.0;
  set vertexArray (ii + 13) 0.0;
  set vertexArray (ii + 14) fsubx;
  set vertexArray (ii + 15) (fsuby +. fsubh);
  set vertexArray (ii + 16) x3;
  set vertexArray (ii + 17) y3;
  set vertexArray (ii + 18) 0.0;
  set vertexArray (ii + 19) 0.0;
  set vertexArray (ii + 20) 0.0;
  set vertexArray (ii + 21) 0.0;
  set vertexArray (ii + 22) (fsubx +. fsubw);
  set vertexArray (ii + 23) fsuby;
  set vertexArray (ii + 24) x4;
  set vertexArray (ii + 25) y4;
  set vertexArray (ii + 26) 0.0;
  set vertexArray (ii + 27) 0.0;
  set vertexArray (ii + 28) 0.0;
  set vertexArray (ii + 29) 0.0;
  set vertexArray (ii + 30) fsubx;
  set vertexArray (ii + 31) fsuby;
  let jj = env.batch.elementPtr;
  let elementArray = env.batch.elementArray;
  set elementArray jj (ii / vertexSize);
  set elementArray (jj + 1) (ii / vertexSize + 1);
  set elementArray (jj + 2) (ii / vertexSize + 2);
  set elementArray (jj + 3) (ii / vertexSize + 1);
  set elementArray (jj + 4) (ii / vertexSize + 2);
  set elementArray (jj + 5) (ii / vertexSize + 3);
  env.batch.vertexPtr = ii + 4 * vertexSize;
  env.batch.elementPtr = jj + 6;
  env.batch.currTex = Some textureBuffer
};


/** Recomputes matrices while resetting size of window */
let resetSize env width height => {
  env.size.width = width;
  env.size.height = height;
  Gl.viewport context::env.gl x::0 y::0 ::width ::height;
  Gl.clearColor context::env.gl r::0. g::0. b::0. a::1.;
  Gl.Mat4.ortho
    out::env.camera.projectionMatrix
    left::0.
    right::(float_of_int width)
    bottom::(float_of_int height)
    top::0.
    near::0.
    far::1.;

  /** Tell OpenGL about what the uniform called `pMatrixUniform` is, here it's the projectionMatrix. **/
  Gl.uniformMatrix4fv
    context::env.gl location::env.pMatrixUniform value::env.camera.projectionMatrix
};
