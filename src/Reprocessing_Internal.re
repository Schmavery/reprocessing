open Reprocessing_Common;

open Reasongl;

module Matrix = Reprocessing_Matrix;

let getProgram =
    (
      ~context,
      ~vertexShader as vertexShaderSource: string,
      ~fragmentShader as fragmentShaderSource: string
    )
    : option(Gl.programT) => {
  let vertexShader = Gl.createShader(~context, RGLConstants.vertex_shader);
  Gl.shaderSource(~context, ~shader=vertexShader, ~source=vertexShaderSource);
  Gl.compileShader(~context, vertexShader);
  let compiledCorrectly =
    Gl.getShaderParameter(~context, ~shader=vertexShader, ~paramName=Gl.Compile_status) == 1;
  if (compiledCorrectly) {
    let fragmentShader = Gl.createShader(~context, RGLConstants.fragment_shader);
    Gl.shaderSource(~context, ~shader=fragmentShader, ~source=fragmentShaderSource);
    Gl.compileShader(~context, fragmentShader);
    let compiledCorrectly =
      Gl.getShaderParameter(~context, ~shader=fragmentShader, ~paramName=Gl.Compile_status) == 1;
    if (compiledCorrectly) {
      let program = Gl.createProgram(~context);
      Gl.attachShader(~context, ~program, ~shader=vertexShader);
      Gl.deleteShader(~context, vertexShader);
      Gl.attachShader(~context, ~program, ~shader=fragmentShader);
      Gl.deleteShader(~context, fragmentShader);
      Gl.linkProgram(~context, program);
      let linkedCorrectly =
        Gl.getProgramParameter(~context, ~program, ~paramName=Gl.Link_status) == 1;
      if (linkedCorrectly) {
        Some(program)
      } else {
        print_endline @@ "Linking error: " ++ Gl.getProgramInfoLog(~context, program);
        None
      }
    } else {
      print_endline @@ "Fragment shader error: " ++ Gl.getShaderInfoLog(~context, fragmentShader);
      None
    }
  } else {
    print_endline @@ "Vertex shader error: " ++ Gl.getShaderInfoLog(~context, vertexShader);
    None
  }
};

let createCanvas = (window) : glEnv => {
  let (width, height) = (Gl.Window.getWidth(window), Gl.Window.getHeight(window));
  let context = Gl.Window.getContext(window);
  Gl.viewport(~context, ~x=(-1), ~y=(-1), ~width, ~height);
  Gl.clearColor(~context, ~r=0., ~g=0., ~b=0., ~a=1.);
  Gl.clear(~context, ~mask=RGLConstants.color_buffer_bit lor RGLConstants.depth_buffer_bit);

  /*** Camera is a simple record containing one matrix used to project a point in 3D onto the screen. **/
  let camera = {projectionMatrix: Gl.Mat4.create()};
  let vertexBuffer = Gl.createBuffer(~context);
  let elementBuffer = Gl.createBuffer(~context);
  let program =
    switch (
      getProgram(
        ~context,
        ~vertexShader=Reprocessing_Shaders.vertexShaderSource,
        ~fragmentShader=Reprocessing_Shaders.fragmentShaderSource
      )
    ) {
    | None => failwith("Could not create the program and/or the shaders. Aborting.")
    | Some(program) => program
    };
  Gl.useProgram(~context, program);

  /*** Get the attribs ahead of time to be used inside the render function **/
  let aVertexPosition = Gl.getAttribLocation(~context, ~program, ~name="aVertexPosition");
  Gl.enableVertexAttribArray(~context, ~attribute=aVertexPosition);
  let aVertexColor = Gl.getAttribLocation(~context, ~program, ~name="aVertexColor");
  Gl.enableVertexAttribArray(~context, ~attribute=aVertexColor);
  let pMatrixUniform = Gl.getUniformLocation(~context, ~program, ~name="uPMatrix");
  Gl.uniformMatrix4fv(~context, ~location=pMatrixUniform, ~value=camera.projectionMatrix);

  /*** Get attribute and uniform locations for later usage in the draw code. **/
  let aTextureCoord = Gl.getAttribLocation(~context, ~program, ~name="aTextureCoord");
  Gl.enableVertexAttribArray(~context, ~attribute=aTextureCoord);

  /*** Generate texture buffer that we'll use to pass image data around. **/
  let texture = Gl.createTexture(~context);

  /*** This tells OpenGL that we're going to be using texture0. OpenGL imposes a limit on the number of
       texture we can manipulate at the same time. That limit depends on the device. We don't care as we'll just
       always use texture0. **/
  Gl.activeTexture(~context, RGLConstants.texture0);

  /*** Bind `texture` to `texture_2d` to modify it's magnification and minification params. **/
  Gl.bindTexture(~context, ~target=RGLConstants.texture_2d, ~texture);
  let uSampler = Gl.getUniformLocation(~context, ~program, ~name="uSampler");

  /*** Load a dummy texture. This is because we're using the same shader for things with and without a texture */
  Gl.texImage2D_RGBA(
    ~context,
    ~target=RGLConstants.texture_2d,
    ~level=0,
    ~width=1,
    ~height=1,
    ~border=0,
    ~data=Gl.Bigarray.of_array(Gl.Bigarray.Uint8, [|0, 0, 0, 0|])
  );
  Gl.texParameteri(
    ~context,
    ~target=RGLConstants.texture_2d,
    ~pname=RGLConstants.texture_mag_filter,
    ~param=RGLConstants.linear
  );
  Gl.texParameteri(
    ~context,
    ~target=RGLConstants.texture_2d,
    ~pname=RGLConstants.texture_min_filter,
    ~param=RGLConstants.linear_mipmap_nearest
  );

  /*** Enable blend and tell OpenGL how to blend. */
  Gl.enable(~context, RGLConstants.blend);
  Gl.blendFunc(~context, RGLConstants.src_alpha, RGLConstants.one_minus_src_alpha);

  /***
   * Will mutate the projectionMatrix to be an ortho matrix with the given boundaries.
   * See this link for quick explanation of what this is.
   * https://shearer12345.github.io/graphics/assets/projectionPerspectiveVSOrthographic.png
   */
  Gl.Mat4.ortho(
    ~out=camera.projectionMatrix,
    ~left=0.,
    ~right=float_of_int(width),
    ~bottom=float_of_int(height),
    ~top=0.,
    ~near=0.,
    ~far=1.
  );
  {
    camera,
    window,
    gl: context,
    batch: {
      vertexArray: Gl.Bigarray.create(Gl.Bigarray.Float32, circularBufferSize * vertexSize),
      elementArray: Gl.Bigarray.create(Gl.Bigarray.Uint16, circularBufferSize),
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
    keyboard: {
      keyCode: Reprocessing_Events.Nothing,
      pressed: Reprocessing_Common.KeySet.empty,
      released: Reprocessing_Common.KeySet.empty,
      down: Reprocessing_Common.KeySet.empty,
    },
    mouse: {pos: (0, 0), prevPos: (0, 0), pressed: false},
    style: {
      fillColor: Some({r: 0., g: 0., b: 0., a: 1.}),
      strokeWeight: 3,
      strokeCap: Round,
      strokeColor: None,
      rectMode: Corner
    },
    styleStack: [],
    matrix: Matrix.createIdentity(),
    matrixStack: [],
    frame: {count: 1, rate: 10, deltaTime: 0.001},
    size: {height, width, resizeable: true}
  }
};

let drawGeometry =
    (
      ~vertexArray: Gl.Bigarray.t(float, Gl.Bigarray.float32_elt),
      ~elementArray: Gl.Bigarray.t(int, Gl.Bigarray.int16_unsigned_elt),
      ~mode,
      ~count,
      ~textureBuffer,
      env
    ) => {
  /* Bind `vertexBuffer`, a pointer to chunk of memory to be sent to the GPU to the "register" called
     `array_buffer` */
  Gl.bindBuffer(~context=env.gl, ~target=RGLConstants.array_buffer, ~buffer=env.vertexBuffer);

  /*** Copy all of the data over into whatever's in `array_buffer` (so here it's `vertexBuffer`) **/
  Gl.bufferData(
    ~context=env.gl,
    ~target=RGLConstants.array_buffer,
    ~data=vertexArray,
    ~usage=RGLConstants.stream_draw
  );

  /*** Tell the GPU about the shader attribute called `aVertexPosition` so it can access the data per vertex */
  Gl.vertexAttribPointer(
    ~context=env.gl,
    ~attribute=env.aVertexPosition,
    ~size=2,
    ~type_=RGLConstants.float_,
    ~normalize=false,
    ~stride=vertexSize * 4,
    ~offset=0
  );

  /*** Same as above but for `aVertexColor` **/
  Gl.vertexAttribPointer(
    ~context=env.gl,
    ~attribute=env.aVertexColor,
    ~size=4,
    ~type_=RGLConstants.float_,
    ~normalize=false,
    ~stride=vertexSize * 4,
    ~offset=2 * 4
  );

  /*** Same as above but for `aTextureCoord` **/
  Gl.vertexAttribPointer(
    ~context=env.gl,
    ~attribute=env.aTextureCoord,
    ~size=2,
    ~type_=RGLConstants.float_,
    ~normalize=false,
    ~stride=vertexSize * 4,
    ~offset=6 * 4
  );

  /*** Tell OpenGL about what the uniform called `uSampler` is pointing at, here it's given 0 which is what
       texture0 represent.  **/
  Gl.uniform1i(~context=env.gl, ~location=env.uSampler, ~value=0);

  /*** Bind `elementBuffer`, a pointer to GPU memory to `element_array_buffer`. That "register" is used for
       the data representing the indices of the vertex. **/
  Gl.bindBuffer(
    ~context=env.gl,
    ~target=RGLConstants.element_array_buffer,
    ~buffer=env.elementBuffer
  );

  /*** Copy the `elementArray` into whatever buffer is in `element_array_buffer` **/
  Gl.bufferData(
    ~context=env.gl,
    ~target=RGLConstants.element_array_buffer,
    ~data=elementArray,
    ~usage=RGLConstants.stream_draw
  );

  /*** We bind `texture` to texture_2d, like we did for the vertex buffers in some ways (I think?) **/
  Gl.bindTexture(~context=env.gl, ~target=RGLConstants.texture_2d, ~texture=textureBuffer);

  /*** Final call which actually tells the GPU to draw. **/
  Gl.drawElements(~context=env.gl, ~mode, ~count, ~type_=RGLConstants.unsigned_short, ~offset=0)
};

/*
 * Helper that will send the currently available data inside globalVertexArray.
 * This function assumes that the vertex data is stored as simple triangles.
 *
 * That function creates a new big array with a new size given the offset and len but does NOT copy the
 * underlying array of memory. So mutation done to that sub array will be reflected in the original one.
 */
let flushGlobalBatch = (env) =>
  if (env.batch.elementPtr > 0) {
    let textureBuffer =
      switch env.batch.currTex {
      | None => env.batch.nullTex
      | Some(textureBuffer) => textureBuffer
      };
    drawGeometry(
      ~vertexArray=Gl.Bigarray.sub(env.batch.vertexArray, ~offset=0, ~len=env.batch.vertexPtr),
      ~elementArray=Gl.Bigarray.sub(env.batch.elementArray, ~offset=0, ~len=env.batch.elementPtr),
      ~mode=RGLConstants.triangles,
      ~count=env.batch.elementPtr,
      ~textureBuffer,
      env
    );
    env.batch.currTex = None;
    env.batch.vertexPtr = 0;
    env.batch.elementPtr = 0
  };

let maybeFlushBatch = (~texture, ~el, ~vert, env) =>
  if (env.batch.elementPtr
      + el >= circularBufferSize
      || env.batch.vertexPtr
      + vert >= circularBufferSize
      || env.batch.elementPtr > 0
      && env.batch.currTex !== texture) {
    flushGlobalBatch(env)
  };

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
let addRectToGlobalBatch =
    (
      env,
      ~bottomRight as (x1, y1),
      ~bottomLeft as (x2, y2),
      ~topRight as (x3, y3),
      ~topLeft as (x4, y4),
      ~color as {r, g, b, a}
    ) => {
  maybeFlushBatch(~texture=None, ~el=6, ~vert=32, env);
  let set = Gl.Bigarray.set;
  let i = env.batch.vertexPtr;
  let vertexArrayToMutate = env.batch.vertexArray;
  set(vertexArrayToMutate, i + 0, x1);
  set(vertexArrayToMutate, i + 1, y1);
  set(vertexArrayToMutate, i + 2, r);
  set(vertexArrayToMutate, i + 3, g);
  set(vertexArrayToMutate, i + 4, b);
  set(vertexArrayToMutate, i + 5, a);
  set(vertexArrayToMutate, i + 6, 0.0);
  set(vertexArrayToMutate, i + 7, 0.0);
  set(vertexArrayToMutate, i + 8, x2);
  set(vertexArrayToMutate, i + 9, y2);
  set(vertexArrayToMutate, i + 10, r);
  set(vertexArrayToMutate, i + 11, g);
  set(vertexArrayToMutate, i + 12, b);
  set(vertexArrayToMutate, i + 13, a);
  set(vertexArrayToMutate, i + 14, 0.0);
  set(vertexArrayToMutate, i + 15, 0.0);
  set(vertexArrayToMutate, i + 16, x3);
  set(vertexArrayToMutate, i + 17, y3);
  set(vertexArrayToMutate, i + 18, r);
  set(vertexArrayToMutate, i + 19, g);
  set(vertexArrayToMutate, i + 20, b);
  set(vertexArrayToMutate, i + 21, a);
  set(vertexArrayToMutate, i + 22, 0.0);
  set(vertexArrayToMutate, i + 23, 0.0);
  set(vertexArrayToMutate, i + 24, x4);
  set(vertexArrayToMutate, i + 25, y4);
  set(vertexArrayToMutate, i + 26, r);
  set(vertexArrayToMutate, i + 27, g);
  set(vertexArrayToMutate, i + 28, b);
  set(vertexArrayToMutate, i + 29, a);
  set(vertexArrayToMutate, i + 30, 0.0);
  set(vertexArrayToMutate, i + 31, 0.0);
  let ii = i / vertexSize;
  let j = env.batch.elementPtr;
  let elementArrayToMutate = env.batch.elementArray;
  set(elementArrayToMutate, j + 0, ii);
  set(elementArrayToMutate, j + 1, ii + 1);
  set(elementArrayToMutate, j + 2, ii + 2);
  set(elementArrayToMutate, j + 3, ii + 1);
  set(elementArrayToMutate, j + 4, ii + 2);
  set(elementArrayToMutate, j + 5, ii + 3);
  env.batch.vertexPtr = i + 4 * vertexSize;
  env.batch.elementPtr = j + 6
};

let drawTriangle = (env, (x1, y1), (x2, y2), (x3, y3), ~color as {r, g, b, a}) => {
  maybeFlushBatch(~texture=None, ~vert=3, ~el=24, env);
  let set = Gl.Bigarray.set;
  let i = env.batch.vertexPtr;
  let vertexArrayToMutate = env.batch.vertexArray;
  set(vertexArrayToMutate, i + 0, x1);
  set(vertexArrayToMutate, i + 1, y1);
  set(vertexArrayToMutate, i + 2, r);
  set(vertexArrayToMutate, i + 3, g);
  set(vertexArrayToMutate, i + 4, b);
  set(vertexArrayToMutate, i + 5, a);
  set(vertexArrayToMutate, i + 6, 0.0);
  set(vertexArrayToMutate, i + 7, 0.0);
  set(vertexArrayToMutate, i + 8, x2);
  set(vertexArrayToMutate, i + 9, y2);
  set(vertexArrayToMutate, i + 10, r);
  set(vertexArrayToMutate, i + 11, g);
  set(vertexArrayToMutate, i + 12, b);
  set(vertexArrayToMutate, i + 13, a);
  set(vertexArrayToMutate, i + 14, 0.0);
  set(vertexArrayToMutate, i + 15, 0.0);
  set(vertexArrayToMutate, i + 16, x3);
  set(vertexArrayToMutate, i + 17, y3);
  set(vertexArrayToMutate, i + 18, r);
  set(vertexArrayToMutate, i + 19, g);
  set(vertexArrayToMutate, i + 20, b);
  set(vertexArrayToMutate, i + 21, a);
  set(vertexArrayToMutate, i + 22, 0.0);
  set(vertexArrayToMutate, i + 23, 0.0);
  let ii = i / vertexSize;
  let j = env.batch.elementPtr;
  let elementArrayToMutate = env.batch.elementArray;
  set(elementArrayToMutate, j + 0, ii);
  set(elementArrayToMutate, j + 1, ii + 1);
  set(elementArrayToMutate, j + 2, ii + 2);
  env.batch.vertexPtr = i + 3 * vertexSize;
  env.batch.elementPtr = j + 3
};

let drawLine = (~p1 as (xx1, yy1), ~p2 as (xx2, yy2), ~color, ~width, ~project, env) => {
  let dx = xx2 -. xx1;
  let dy = yy2 -. yy1;
  let mag = sqrt(dx *. dx +. dy *. dy);
  let radius = width /. 2.;
  let xthing = dy /. mag *. radius;
  let ything = -. dx /. mag *. radius;
  let (projectx, projecty) = project ? (dx /. mag *. radius, xthing) : (0., 0.);
  let x1 = xx2 +. xthing +. projectx;
  let y1 = yy2 +. ything +. projecty;
  let x2 = xx1 +. xthing -. projectx;
  let y2 = yy1 +. ything -. projecty;
  let x3 = xx2 -. xthing +. projectx;
  let y3 = yy2 -. ything +. projecty;
  let x4 = xx1 -. xthing -. projectx;
  let y4 = yy1 -. ything -. projecty;
  addRectToGlobalBatch(
    env,
    ~bottomRight=(x1, y1),
    ~bottomLeft=(x2, y2),
    ~topRight=(x3, y3),
    ~topLeft=(x4, y4),
    ~color
  )
};

let drawArc =
    (
      env,
      (xCenterOfCircle: float, yCenterOfCircle: float),
      radx: float,
      rady: float,
      start: float,
      stop: float,
      isPie: bool,
      matrix: array(float),
      {r, g, b, a}
    ) => {
  let transform = Matrix.matptmul(matrix);
  let noOfFans = int_of_float(radx +. rady) / 4 + 10;
  maybeFlushBatch(~texture=None, ~vert=8 * noOfFans, ~el=3 * noOfFans, env);
  let pi = 4.0 *. atan(1.0);
  let anglePerFan = 2. *. pi /. float_of_int(noOfFans);
  let verticesData = env.batch.vertexArray;
  let elementData = env.batch.elementArray;
  let set = Gl.Bigarray.set;
  let get = Gl.Bigarray.get;
  let vertexArrayOffset = env.batch.vertexPtr;
  let elementArrayOffset = env.batch.elementPtr;
  let start_i =
    if (isPie) {
      /* Start one earlier and force the first point to be the center */
      int_of_float(start /. anglePerFan)
      - 2
    } else {
      int_of_float(start /. anglePerFan) - 1
    };
  let stop_i = int_of_float(stop /. anglePerFan) - 1;
  for (i in start_i to stop_i) {
    let (xCoordinate, yCoordinate) =
      transform(
        if (isPie && i - start_i == 0) {
          (
            /* force the first point to be the center */
            xCenterOfCircle,
            yCenterOfCircle
          )
        } else {
          let angle = anglePerFan *. float_of_int(i + 1);
          (xCenterOfCircle +. cos(angle) *. radx, yCenterOfCircle +. sin(angle) *. rady)
        }
      );
    let ii = (i - start_i) * vertexSize + vertexArrayOffset;
    set(verticesData, ii + 0, xCoordinate);
    set(verticesData, ii + 1, yCoordinate);
    set(verticesData, ii + 2, r);
    set(verticesData, ii + 3, g);
    set(verticesData, ii + 4, b);
    set(verticesData, ii + 5, a);
    set(verticesData, ii + 6, 0.0);
    set(verticesData, ii + 7, 0.0);
    /* For the first three vertices, we don't do any deduping. Then for the subsequent ones, we'll actually
       have 3 elements, one pointing at the first vertex, one pointing at the previously added vertex and one
       pointing at the current vertex. This mimicks the behavior of triangle_fan. */
    if (i - start_i < 3) {
      set(elementData, i - start_i + elementArrayOffset, ii / vertexSize)
    } else {
      /* We've already added 3 elements, for i = 0, 1 and 2. From now on, we'll add 3 elements _per_ i.
         To calculate the correct offset in `elementData` we remove 3 from i as if we're starting from 0 (the
         first time we enter this loop i = 3), then for each i we'll add 3 elements (so multiply by 3) BUT for
         i = 3 we want `jj` to be 3 so we shift everything by 3 (so add 3). Everything's also shifted by
         `elementArrayOffset` */
      let jj = (i - start_i - 3) * 3 + elementArrayOffset + 3;
      set(elementData, jj, vertexArrayOffset / vertexSize);
      set(elementData, jj + 1, get(elementData, jj - 1));
      set(elementData, jj + 2, ii / vertexSize)
    }
  };
  env.batch.vertexPtr = env.batch.vertexPtr + noOfFans * vertexSize;
  env.batch.elementPtr = env.batch.elementPtr + (stop_i - start_i - 3) * 3 + 3
};

let drawEllipse = (env, center, radx: float, rady: float, matrix: array(float), c) =>
  drawArc(env, center, radx, rady, 0., Reprocessing_Constants.tau, false, matrix, c);

let drawArcStroke =
    (
      env,
      (xCenterOfCircle: float, yCenterOfCircle: float),
      radx: float,
      rady: float,
      start: float,
      stop: float,
      isOpen: bool,
      isPie: bool,
      matrix: array(float),
      {r, g, b, a} as strokeColor,
      strokeWidth
    ) => {
  let transform = Matrix.matptmul(matrix);
  let verticesData = env.batch.vertexArray;
  let elementData = env.batch.elementArray;
  let noOfFans = int_of_float(radx +. rady) / 4 + 10;
  let set = Gl.Bigarray.set;
  maybeFlushBatch(~texture=None, ~vert=16, ~el=6, env);
  let pi = 4.0 *. atan(1.0);
  let anglePerFan = 2. *. pi /. float_of_int(noOfFans);
  /* I calculated this roughly by doing:
     anglePerFan *. float_of_int (i + 1) == start
     i+1 == start /. anglePerFan
     */
  let start_i = int_of_float(start /. anglePerFan) - 1;
  let stop_i = int_of_float(stop /. anglePerFan) - 1;
  let prevEl: ref(option((int, int))) = ref(None);
  let halfwidth = float_of_int(strokeWidth) /. 2.;
  for (i in start_i to stop_i) {
    let angle = anglePerFan *. float_of_int(i + 1);
    let (xCoordinateInner, yCoordinateInner) =
      transform((
        xCenterOfCircle +. cos(angle) *. (radx -. halfwidth),
        yCenterOfCircle +. sin(angle) *. (rady -. halfwidth)
      ));
    let (xCoordinateOuter, yCoordinateOuter) =
      transform((
        xCenterOfCircle +. cos(angle) *. (radx +. halfwidth),
        yCenterOfCircle +. sin(angle) *. (rady +. halfwidth)
      ));
    let ii = env.batch.vertexPtr;
    set(verticesData, ii + 0, xCoordinateInner);
    set(verticesData, ii + 1, yCoordinateInner);
    set(verticesData, ii + 2, r);
    set(verticesData, ii + 3, g);
    set(verticesData, ii + 4, b);
    set(verticesData, ii + 5, a);
    set(verticesData, ii + 6, 0.0);
    set(verticesData, ii + 7, 0.0);
    let ii = ii + vertexSize;
    set(verticesData, ii + 0, xCoordinateOuter);
    set(verticesData, ii + 1, yCoordinateOuter);
    set(verticesData, ii + 2, r);
    set(verticesData, ii + 3, g);
    set(verticesData, ii + 4, b);
    set(verticesData, ii + 5, a);
    set(verticesData, ii + 6, 0.0);
    set(verticesData, ii + 7, 0.0);
    env.batch.vertexPtr = env.batch.vertexPtr + vertexSize * 2;
    let currOuter = ii / vertexSize;
    let currInner = ii / vertexSize - 1;
    let currEl = Some((currInner, currOuter));
    switch prevEl^ {
    | None => prevEl := currEl
    | Some((prevInner, prevOuter)) =>
      let elementArrayOffset = env.batch.elementPtr;
      set(elementData, elementArrayOffset, prevInner);
      set(elementData, elementArrayOffset + 1, prevOuter);
      set(elementData, elementArrayOffset + 2, currOuter);
      set(elementData, elementArrayOffset + 3, currOuter);
      set(elementData, elementArrayOffset + 4, prevInner);
      set(elementData, elementArrayOffset + 5, currInner);
      env.batch.elementPtr = env.batch.elementPtr + 6;
      prevEl := currEl
    }
  };
  if (! isOpen) {
    let (startX, startY) =
      transform((xCenterOfCircle +. cos(start) *. radx, yCenterOfCircle +. sin(start) *. rady));
    let (stopX, stopY) =
      transform((xCenterOfCircle +. cos(stop) *. radx, yCenterOfCircle +. sin(stop) *. rady));
    if (isPie) {
      drawLine(
        ~p1=(startX, startY),
        ~p2=(xCenterOfCircle, yCenterOfCircle),
        ~color=strokeColor,
        ~width=halfwidth,
        ~project=false,
        env
      );
      drawLine(
        ~p1=(stopX, stopY),
        ~p2=(xCenterOfCircle, yCenterOfCircle),
        ~color=strokeColor,
        ~width=halfwidth,
        ~project=false,
        env
      );
      drawEllipse(
        env,
        transform((xCenterOfCircle, yCenterOfCircle)),
        halfwidth,
        halfwidth,
        matrix,
        strokeColor
      )
    } else {
      drawLine(
        ~p1=(startX, startY),
        ~p2=(stopX, stopY),
        ~color=strokeColor,
        ~width=halfwidth,
        ~project=false,
        env
      )
    };
    drawEllipse(env, (startX, startY), halfwidth, halfwidth, matrix, strokeColor);
    drawEllipse(env, (stopX, stopY), halfwidth, halfwidth, matrix, strokeColor)
  }
};

let loadImage = (env: glEnv, filename, isPixel) : imageT => {
  let imageRef = ref(None);
  Gl.loadImage(
    ~filename,
    ~callback=
      (imageData) =>
        switch imageData {
        | None => failwith("Could not load image '" ++ (filename ++ "'.")) /* TODO: handle this better? */
        | Some(img) =>
          let env = env;
          let textureBuffer = Gl.createTexture(~context=env.gl);
          let height = Gl.getImageHeight(img);
          let width = Gl.getImageWidth(img);
          let filter = isPixel ? Constants.nearest : Constants.linear;
          imageRef := Some({img, textureBuffer, height, width});
          Gl.bindTexture(~context=env.gl, ~target=Constants.texture_2d, ~texture=textureBuffer);
          Gl.texImage2DWithImage(
            ~context=env.gl,
            ~target=Constants.texture_2d,
            ~level=0,
            ~image=img
          );
          Gl.texParameteri(
            ~context=env.gl,
            ~target=Constants.texture_2d,
            ~pname=Constants.texture_mag_filter,
            ~param=filter
          );
          Gl.texParameteri(
            ~context=env.gl,
            ~target=Constants.texture_2d,
            ~pname=Constants.texture_min_filter,
            ~param=filter
          );
          Gl.texParameteri(
            ~context=env.gl,
            ~target=Constants.texture_2d,
            ~pname=Constants.texture_wrap_s,
            ~param=Constants.clamp_to_edge
          );
          Gl.texParameteri(
            ~context=env.gl,
            ~target=Constants.texture_2d,
            ~pname=Constants.texture_wrap_t,
            ~param=Constants.clamp_to_edge
          )
        },
    ()
  );
  imageRef
};

let drawImage =
    (
      {width: imgw, height: imgh, textureBuffer},
      ~p1 as (x1, y1),
      ~p2 as (x2, y2),
      ~p3 as (x3, y3),
      ~p4 as (x4, y4),
      ~subx,
      ~suby,
      ~subw,
      ~subh,
      env
    ) => {
  maybeFlushBatch(~texture=Some(textureBuffer), ~vert=32, ~el=6, env);
  let (fsubx, fsuby, fsubw, fsubh) = (
    float_of_int(subx) /. float_of_int(imgw),
    float_of_int(suby) /. float_of_int(imgh),
    float_of_int(subw) /. float_of_int(imgw),
    float_of_int(subh) /. float_of_int(imgh)
  );
  let set = Gl.Bigarray.set;
  let ii = env.batch.vertexPtr;
  let vertexArray = env.batch.vertexArray;
  set(vertexArray, ii + 0, x1);
  set(vertexArray, ii + 1, y1);
  set(vertexArray, ii + 2, 0.0);
  set(vertexArray, ii + 3, 0.0);
  set(vertexArray, ii + 4, 0.0);
  set(vertexArray, ii + 5, 0.0);
  set(vertexArray, ii + 6, fsubx +. fsubw);
  set(vertexArray, ii + 7, fsuby +. fsubh);
  set(vertexArray, ii + 8, x2);
  set(vertexArray, ii + 9, y2);
  set(vertexArray, ii + 10, 0.0);
  set(vertexArray, ii + 11, 0.0);
  set(vertexArray, ii + 12, 0.0);
  set(vertexArray, ii + 13, 0.0);
  set(vertexArray, ii + 14, fsubx);
  set(vertexArray, ii + 15, fsuby +. fsubh);
  set(vertexArray, ii + 16, x3);
  set(vertexArray, ii + 17, y3);
  set(vertexArray, ii + 18, 0.0);
  set(vertexArray, ii + 19, 0.0);
  set(vertexArray, ii + 20, 0.0);
  set(vertexArray, ii + 21, 0.0);
  set(vertexArray, ii + 22, fsubx +. fsubw);
  set(vertexArray, ii + 23, fsuby);
  set(vertexArray, ii + 24, x4);
  set(vertexArray, ii + 25, y4);
  set(vertexArray, ii + 26, 0.0);
  set(vertexArray, ii + 27, 0.0);
  set(vertexArray, ii + 28, 0.0);
  set(vertexArray, ii + 29, 0.0);
  set(vertexArray, ii + 30, fsubx);
  set(vertexArray, ii + 31, fsuby);
  let jj = env.batch.elementPtr;
  let elementArray = env.batch.elementArray;
  set(elementArray, jj, ii / vertexSize);
  set(elementArray, jj + 1, ii / vertexSize + 1);
  set(elementArray, jj + 2, ii / vertexSize + 2);
  set(elementArray, jj + 3, ii / vertexSize + 1);
  set(elementArray, jj + 4, ii / vertexSize + 2);
  set(elementArray, jj + 5, ii / vertexSize + 3);
  env.batch.vertexPtr = ii + 4 * vertexSize;
  env.batch.elementPtr = jj + 6;
  env.batch.currTex = Some(textureBuffer)
};

let drawImageWithMatrix = (image, ~x, ~y, ~width, ~height, ~subx, ~suby, ~subw, ~subh, env) => {
  let transform = Matrix.matptmul(env.matrix);
  let p1 = transform((float_of_int @@ x + width, float_of_int @@ y + height));
  let p2 = transform((float_of_int(x), float_of_int @@ y + height));
  let p3 = transform((float_of_int @@ x + width, float_of_int(y)));
  let p4 = transform((float_of_int(x), float_of_int(y)));
  drawImage(image, ~p1, ~p2, ~p3, ~p4, ~subx, ~suby, ~subw, ~subh, env)
};


/*** Recomputes matrices while resetting size of window */
let resetSize = (env, width, height) => {
  env.size.width = width;
  env.size.height = height;
  let (pixelWidth, pixelHeight) =
    Gl.Window.(getPixelWidth(env.window), getPixelHeight(env.window));
  Gl.viewport(~context=env.gl, ~x=0, ~y=0, ~width=pixelWidth, ~height=pixelHeight);
  Gl.clearColor(~context=env.gl, ~r=0., ~g=0., ~b=0., ~a=1.);
  Gl.Mat4.ortho(
    ~out=env.camera.projectionMatrix,
    ~left=0.,
    ~right=float_of_int(width),
    ~bottom=float_of_int(height),
    ~top=0.,
    ~near=0.,
    ~far=1.
  );

  /*** Tell OpenGL about what the uniform called `pMatrixUniform` is, here it's the projectionMatrix. **/
  Gl.uniformMatrix4fv(
    ~context=env.gl,
    ~location=env.pMatrixUniform,
    ~value=env.camera.projectionMatrix
  )
};
