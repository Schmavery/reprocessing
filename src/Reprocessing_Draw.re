open Reprocessing_Common;

module Internal = Reprocessing_Internal;

module Matrix = Reprocessing_Matrix;

module Env = Reprocessing_Env;

module Font = Reprocessing_Font.Font;

let translate ::x ::y env =>
  Matrix.(matmatmul env.matrix (createTranslation x y));

let rotate theta env => Matrix.(matmatmul env.matrix (createRotation theta));

let fill color (env: glEnv) =>
  env.style = {...env.style, fillColor: Some color};

let noFill (env: glEnv) => env.style = {...env.style, fillColor: None};

let stroke color env => env.style = {...env.style, strokeColor: Some color};

let noStroke env => env.style = {...env.style, strokeColor: None};

let strokeWeight weight env =>
  env.style = {...env.style, strokeWeight: weight};

let strokeCap cap env => env.style = {...env.style, strokeCap: cap};

let pushStyle env => env.styleStack = [env.style, ...env.styleStack];

let popStyle env =>
  switch env.styleStack {
  /* Matches Processing error message */
  | [] => failwith "Too many `popStyle` without enough `pushStyle`."
  | [hd, ...tl] =>
    env.style = hd;
    env.styleStack = tl
  };

let loadImage ::filename env => Internal.loadImage env filename;

let image img pos::(x, y) (env: glEnv) =>
  switch !img {
  | None => print_endline "image not ready yet, just doing nothing :D"
  | Some ({width, height} as i) =>
    Internal.drawImage i ::x ::y subx::0 suby::0 subw::width subh::height env
  };

let linef ::p1 ::p2 (env: glEnv) =>
  switch env.style.strokeColor {
  | None => () /* don't draw stroke */
  | Some color =>
    let transform = Matrix.matptmul env.matrix;
    let width = float_of_int env.style.strokeWeight;
    let radius = width /. 2.;
    let project = env.style.strokeCap == Project;
    Internal.drawLine
      p1::(transform p1) p2::(transform p2) ::color ::width ::project env;
    if (env.style.strokeCap == Round) {
      Internal.drawEllipse env p1 radius radius env.matrix color;
      Internal.drawEllipse env p2 radius radius env.matrix color
    }
  };

let line p1::(x1, y1) p2::(x2, y2) (env: glEnv) =>
  linef
    p1::(float_of_int x1, float_of_int y1)
    p2::(float_of_int x2, float_of_int y2)
    env;

let ellipsef ::center ::radx ::rady (env: glEnv) => {
  switch env.style.fillColor {
  | None => () /* Don't draw fill */
  | Some fill => Internal.drawEllipse env center radx rady env.matrix fill
  };
  switch env.style.strokeColor {
  | None => () /* Don't draw stroke */
  | Some stroke =>
    Internal.drawArcStroke
      env
      center
      radx
      rady
      0.
      Reprocessing_Constants.tau
      false
      false
      env.matrix
      stroke
      env.style.strokeWeight
  }
};

let ellipse center::(cx, cy) ::radx ::rady (env: glEnv) =>
  ellipsef
    center::(float_of_int cx, float_of_int cy)
    radx::(float_of_int radx)
    rady::(float_of_int rady)
    env;

let quadf ::p1 ::p2 ::p3 ::p4 (env: glEnv) => {
  let transform = Matrix.matptmul env.matrix;
  let (p1, p2, p3, p4) = (
    transform p1,
    transform p2,
    transform p3,
    transform p4
  );
  switch env.style.fillColor {
  | None => () /* Don't draw fill */
  | Some fill =>
    Internal.addRectToGlobalBatch
      env topLeft::p1 topRight::p2 bottomRight::p3 bottomLeft::p4 color::fill
  };
  switch env.style.strokeColor {
  | None => () /* don't draw stroke */
  | Some color =>
    linef ::p1 ::p2 env;
    linef p1::p2 p2::p3 env;
    linef p1::p3 p2::p4 env;
    linef p1::p4 p2::p1 env;
    let r = float_of_int env.style.strokeWeight /. 2.;
    let m = Matrix.identity;
    Internal.drawEllipse env p1 r r m color;
    Internal.drawEllipse env p2 r r m color;
    Internal.drawEllipse env p3 r r m color;
    Internal.drawEllipse env p4 r r m color
  }
};

let quad p1::(x1, y1) p2::(x2, y2) p3::(x3, y3) p4::(x4, y4) (env: glEnv) =>
  quadf
    p1::(float_of_int x1, float_of_int y1)
    p2::(float_of_int x2, float_of_int y2)
    p3::(float_of_int x3, float_of_int y3)
    p4::(float_of_int x4, float_of_int y4)
    env;

let rectf pos::(x, y) ::width ::height (env: glEnv) =>
  quadf
    p1::(x, y)
    p2::(x +. width, y)
    p3::(x +. width, y +. height)
    p4::(x, y +. height)
    env;

let rect pos::(x, y) ::width ::height (env: glEnv) =>
  rectf
    pos::(float_of_int x, float_of_int y)
    width::(float_of_int width)
    height::(float_of_int height)
    env;

let bezier
    p1::(xx1, yy1)
    p2::(xx2, yy2)
    p3::(xx3, yy3)
    p4::(xx4, yy4)
    (env: glEnv) => {
  let bezier_point t => (
    (1. -. t) *\* 3. *. xx1 +. 3. *. (1. -. t) *\* 2. *. t *. xx2 +.
    3. *. (1. -. t) *. t *\* 2. *. xx3 +.
    t *\* 3. *. xx4,
    (1. -. t) *\* 3. *. yy1 +. 3. *. (1. -. t) *\* 2. *. t *. yy2 +.
    3. *. (1. -. t) *. t *\* 2. *. yy3 +.
    t *\* 3. *. yy4
  );
  for i in 0 to 99 {
    linef
      p1::(bezier_point (float_of_int i /. 100.0))
      p2::(bezier_point (float_of_int (i + 1) /. 100.0))
      env
  }
};

let pixelf pos::(x, y) ::color (env: glEnv) => {
  let w = float_of_int env.style.strokeWeight;
  Internal.addRectToGlobalBatch
    env
    bottomRight::(x +. w, y +. w)
    bottomLeft::(x, y +. w)
    topRight::(x +. w, y)
    topLeft::(x, y)
    ::color
};

let pixel pos::(x, y) ::color (env: glEnv) =>
  pixelf pos::(float_of_int x, float_of_int y) ::color env;

let trianglef ::p1 ::p2 ::p3 (env: glEnv) => {
  let transform = Matrix.matptmul env.matrix;
  let (p1, p2, p3) = (transform p1, transform p2, transform p3);
  switch env.style.fillColor {
  | None => () /* don't draw fill */
  | Some color => Internal.drawTriangle env p1 p2 p3 ::color
  };
  switch env.style.strokeColor {
  | None => () /* don't draw stroke */
  | Some color =>
    linef ::p1 ::p2 env;
    linef p1::p2 p2::p3 env;
    linef p1::p3 p2::p1 env;
    let r = float_of_int env.style.strokeWeight /. 2.;
    let m = Matrix.identity;
    Internal.drawEllipse env p1 r r m color;
    Internal.drawEllipse env p2 r r m color;
    Internal.drawEllipse env p3 r r m color
  }
};

let triangle p1::(x1, y1) p2::(x2, y2) p3::(x3, y3) (env: glEnv) =>
  trianglef
    p1::(float_of_int x1, float_of_int y1)
    p2::(float_of_int x2, float_of_int y2)
    p3::(float_of_int x3, float_of_int y3)
    env;

let arcf ::center ::radx ::rady ::start ::stop ::isOpen ::isPie (env: glEnv) => {
  switch env.style.fillColor {
  | None => () /* don't draw fill */
  | Some color =>
    Internal.drawArc env center radx rady start stop isPie env.matrix color
  };
  switch env.style.strokeColor {
  | None => () /* don't draw stroke */
  | Some stroke =>
    Internal.drawArcStroke
      env
      center
      radx
      rady
      start
      stop
      isOpen
      isPie
      env.matrix
      stroke
      env.style.strokeWeight
  }
};

let arc
    center::(cx, cy)
    ::radx
    ::rady
    ::start
    ::stop
    ::isOpen
    ::isPie
    (env: glEnv) =>
  arcf
    center::(float_of_int cx, float_of_int cy)
    radx::(float_of_int radx)
    rady::(float_of_int rady)
    ::start
    ::stop
    ::isOpen
    ::isPie
    env;

let loadFont ::filename (env: glEnv) => Font.parseFontFormat env filename;

let text ::font ::body pos::(x, y) (env: glEnv) =>
  Font.drawString env font body x y;

let clear env =>
  Reasongl.Gl.clear
    context::env.gl
    mask::(Constants.color_buffer_bit lor Constants.depth_buffer_bit);

let background color (env: glEnv) => {
  let w = float_of_int (Env.width env);
  let h = float_of_int (Env.height env);
  Internal.addRectToGlobalBatch
    env
    bottomRight::(w, h)
    bottomLeft::(0., h)
    topRight::(w, 0.)
    topLeft::(0., 0.)
    ::color
};
