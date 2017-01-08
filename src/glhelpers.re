/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
open Common;

open Glloader;

open Utils;

let circularBufferSize = 6 * 10000;

let globalVertexArray = ref (Gl.Bigarray.create Gl.Bigarray.Float32 (circularBufferSize * 9));

let globalElementArray = ref (Gl.Bigarray.create Gl.Bigarray.Uint16 circularBufferSize);

let vertexArrayPtr = ref 0;

let elementArrayPtr = ref 0;

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
  let elementBuffer = Gl.createBuffer gl;
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

  /** Load a dummy texture. This is because we're using the same shader for things with and without a texture
      The shader will try make lookups to get some color from the texture, but then that color will be multiplied
      by `uTextureFlag`, which would be 0 in the case where we're rendering a colored shape. */
  Gl.texImage2D
    context::gl
    target::Constants.texture_2d
    level::0
    internalFormat::Constants.rgba
    width::1
    height::1
    format::Constants.rgba
    type_::Constants.unsigned_byte
    data::(Gl.toTextureData [|0, 0, 0, 0|]);
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
    elementBuffer,
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
 * The vertex array looks like:
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
 *
 *
 * The element array is just an array of indices of vertices given that each vertex takes 9 * 4 bytes.
 * For example, if the element array looks like [|0, 1, 2, 1, 2, 3|], we're telling the GPU to draw 2
 * triangles: one with the vertices 0, 1 and 2 from the vertex array, and one with the vertices 1, 2 and 3.
 * We can "point" to duplicated vertices in our geometry to avoid sending those vertices.
 */
let addRectToGlobalBatch (x1, y1) (x2, y2) (x3, y3) (x4, y4) {r, g, b} => {
  let set = Gl.Bigarray.set;
  let toColorFloat i => float_of_int i /. 255.;
  let (r, g, b) = (toColorFloat r, toColorFloat g, toColorFloat b);
  let i = !vertexArrayPtr;
  let vertexArrayToMutate = !globalVertexArray;
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
  vertexArrayPtr := i + 36;
  let ii = i / 9;
  let j = !elementArrayPtr;
  let elementArrayToMutate = !globalElementArray;
  set elementArrayToMutate (j + 0) ii;
  set elementArrayToMutate (j + 1) (ii + 1);
  set elementArrayToMutate (j + 2) (ii + 2);
  set elementArrayToMutate (j + 3) (ii + 1);
  set elementArrayToMutate (j + 4) (ii + 2);
  set elementArrayToMutate (j + 5) (ii + 3);
  elementArrayPtr := j + 6
};

let drawGeometry
    vertexArray::(vertexArray: Gl.Bigarray.t float Gl.Bigarray.float32_elt)
    elementArray::(elementArray: Gl.Bigarray.t int Gl.Bigarray.int16_unsigned_elt)
    ::mode
    ::count
    ::textureFlag=0.
    ::textureBuffer=?
    env => {
  let textureBuffer =
    switch textureBuffer {
    | None => env.texture
    | Some textureBuffer => textureBuffer
    };
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

  /** Bind `elementBuffer`, a pointer to GPU memory to `element_array_buffer`. That "register" is used for
      the data representing the indices of the vertex. **/
  Gl.bindBuffer context::env.gl target::Constants.element_array_buffer buffer::env.elementBuffer;

  /** Copy the `elementArray` into whatever buffer is in `element_array_buffer` **/
  Gl.bufferData
    context::env.gl
    target::Constants.element_array_buffer
    data::elementArray
    usage::Constants.stream_draw;

  /** This tells OpenGL that we're going to be using texture0. OpenGL imposes a limit on the number of
      texture we can manipulate at the same time. That limit depends on the device. We don't care as we'll just
      always use texture0. **/
  Gl.activeTexture context::env.gl target::Constants.texture0;

  /** We bind `texture` to texture_2d, like we did for the vertex buffers in some ways (I think?) **/
  Gl.bindTexture context::env.gl target::Constants.texture_2d texture::textureBuffer;

  /** Tell OpenGL about what the uniform called `uSampler` is pointing at, here it's given 0 which is what
      texture0 represent. **/
  Gl.uniform1i context::env.gl location::env.uSampler 0;

  /** Tell OpenGL about what the uniform called `pMatrixUniform` is, here it's the projectionMatrix. **/
  Gl.uniformMatrix4fv
    context::env.gl location::env.pMatrixUniform value::env.camera.projectionMatrix;

  /** Last uniform, the `uTextureFlag` which allows us to only have one shader and flip flop between using
      a color or a texture to fill the geometry. **/
  Gl.uniform1f context::env.gl location::env.uTextureFlag textureFlag;

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
  if (!elementArrayPtr > 0) {
    drawGeometry
      vertexArray::(Gl.Bigarray.sub !globalVertexArray offset::0 len::!vertexArrayPtr)
      elementArray::(Gl.Bigarray.sub !globalElementArray offset::0 len::!elementArrayPtr)
      mode::Constants.triangles
      count::!elementArrayPtr
      !env;
    vertexArrayPtr := 0;
    elementArrayPtr := 0
  };

let drawEllipseInternal env xCenterOfCircle yCenterOfCircle radx rady => {
  let noOfFans = (radx + rady) * 2;
  assert ((noOfFans - 2) * 3 < circularBufferSize);
  let pi = 4.0 *. atan 1.0;
  let anglePerFan = 2. *. pi /. float_of_int noOfFans;
  let radxf = float_of_int radx;
  let radyf = float_of_int rady;
  let toColorFloat i => float_of_int i /. 255.;
  let (r, g, b) = (
    toColorFloat (!env).currFill.r,
    toColorFloat (!env).currFill.g,
    toColorFloat (!env).currFill.b
  );
  let xCenterOfCirclef = float_of_int xCenterOfCircle;
  let yCenterOfCirclef = float_of_int yCenterOfCircle;
  /* If there isn't enough space to add the ellipse, we flush the global buffer. */
  if (circularBufferSize - !elementArrayPtr < (noOfFans - 2) * 3) {
    flushGlobalBatch env
  };
  let verticesData = !globalVertexArray;
  let elementData = !globalElementArray;
  let set = Gl.Bigarray.set;
  let get = Gl.Bigarray.get;
  let vertexArrayOffset = !vertexArrayPtr;
  let elementArrayOffset = !elementArrayPtr;
  for i in 0 to (noOfFans - 1) {
    let angle = anglePerFan *. float_of_int (i + 1);
    let xCoordinate = xCenterOfCirclef +. cos angle *. radxf;
    let yCoordinate = yCenterOfCirclef +. sin angle *. radyf;
    let ii = i * 9 + vertexArrayOffset;
    set verticesData (ii + 0) xCoordinate;
    set verticesData (ii + 1) yCoordinate;
    set verticesData (ii + 2) 0.0;
    set verticesData (ii + 3) r;
    set verticesData (ii + 4) g;
    set verticesData (ii + 5) b;
    set verticesData (ii + 6) 1.0;
    set verticesData (ii + 7) 0.0;
    set verticesData (ii + 8) 0.0;
    /* For the first three vertices, we don't do any deduping. Then for the subsequent ones, we'll actually
       have 3 elements, one pointing at the first vertex, one poiting at the previously added vertex and one
       pointing at the current vertex. This mimicks the behavior of triangle_fan. */
    if (i < 3) {
      set elementData (i + elementArrayOffset) (ii / 9)
    } else {
      /* We've already added 3 elements, for i = 0, 1 and 2. From now on, we'll add 3 elements _per_ i.
         To caculate the correct offset in `elementData` we remove 3 from i as if we're starting from 0 (the
         first time we enter this loop i = 3), then for each i we'll add 3 elements (so multiply by 3) BUT for
         i = 3 we want `jj` to be 3 so we shift everything by 3 (so add 3). Everything's also shifted by
         `elementArrayOffset` */
      let jj = (i - 3) * 3 + elementArrayOffset + 3;
      set elementData jj (vertexArrayOffset / 9);
      set elementData (jj + 1) (get elementData (jj - 1));
      set elementData (jj + 2) (ii / 9)
    }
  };
  vertexArrayPtr := !vertexArrayPtr + noOfFans * 9;
  elementArrayPtr := !elementArrayPtr + (noOfFans - 3) * 3 + 3
};

let loadImage (env: ref glEnv) filename :imageT => {
  let imageRef = ref None;
  Gl.loadImage
    ::filename
    callback::(
      fun imageData =>
        switch imageData {
        | None => failwith ("Could not load image '" ^ filename ^ "'.") /* TODO: handle this better? */
        | Some img =>
          let env = !env;
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

let addTextureToGlobalBatch {width, height} ::x ::y ::subx ::suby ::subw ::subh => {
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
  let ii = !vertexArrayPtr;
  let vertexArray = !globalVertexArray;
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
  vertexArrayPtr := ii + 36;
  let jj = !elementArrayPtr;
  let elementArray = !globalElementArray;
  set elementArray jj (ii / 9);
  set elementArray (jj + 1) (ii / 9 + 1);
  set elementArray (jj + 2) (ii / 9 + 2);
  set elementArray (jj + 3) (ii / 9 + 1);
  set elementArray (jj + 4) (ii / 9 + 2);
  set elementArray (jj + 5) (ii / 9 + 3);
  elementArrayPtr := jj + 6
};

let flushGlobalBatchWithTexture (env: ref glEnv) {textureBuffer} => {
  drawGeometry
    vertexArray::(Gl.Bigarray.sub !globalVertexArray offset::0 len::!vertexArrayPtr)
    elementArray::(Gl.Bigarray.sub !globalElementArray offset::0 len::!elementArrayPtr)
    mode::Constants.triangles
    count::!elementArrayPtr
    textureFlag::1.0
    ::textureBuffer
    !env;
  vertexArrayPtr := 0;
  elementArrayPtr := 0
};

let drawImageInternal
    (env: ref glEnv)
    {img, textureBuffer, width, height}
    ::x
    ::y
    ::subx
    ::suby
    ::subw
    ::subh => {
  let env = !env;
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
  let verticesColorAndTexture = [|
    x1,
    y1,
    0.0,
    0.0,
    0.0,
    0.0,
    1.,
    fsubx +. fsubw,
    fsuby +. fsubh,
    x2,
    y2,
    0.0,
    0.0,
    0.0,
    0.0,
    1.,
    fsubx,
    fsuby +. fsubh,
    x3,
    y3,
    0.0,
    0.0,
    0.0,
    0.0,
    1.,
    fsubx +. fsubw,
    fsuby,
    x4,
    y4,
    0.0,
    0.0,
    0.0,
    0.0,
    1.,
    fsubx,
    fsuby
  |];
  drawGeometry
    vertexArray::(Gl.Bigarray.of_array Gl.Bigarray.Float32 verticesColorAndTexture)
    elementArray::(Gl.Bigarray.of_array Gl.Bigarray.Uint16 [|0, 1, 2, 1, 2, 3|])
    mode::Constants.triangles
    count::6
    textureFlag::1.0
    ::textureBuffer
    env
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
