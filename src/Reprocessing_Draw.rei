/** Specifies an amount to displace objects within the display window.
   * The dx parameter specifies left/right translation, the dy parameter
   * specifies up/down translation.
   *
   * Transformations are cumulative and apply to everything that happens
   * after and subsequent calls to the function accumulates the effect.
   * For example, calling `translate dx::50 dy::0 env` and then
   * `translate dx::20 dy::0 env` is the same as `translate dx::70 dy::0 env`.
   * If `translate` is called within `draw`, the transformation is reset
   * when the loop begins again. This function can be further controlled
   * by using `pushMatrix` and `popMatrix`.
 */
let translate: x::float => y::float => Reprocessing_Types.Types.glEnvT => unit;


/** Rotates the amount specified by the angle parameter. Angles must be
 * specified in radians (values from 0 to two_pi), or they can be converted
 * from degrees to radians with the `radians` function.

 * The coordinates are always rotated around their relative position to the
 * origin. Positive numbers rotate objects in a clockwise direction and
 * negative numbers rotate in the couterclockwise direction. Transformations
 * apply to everything that happens afterward, and subsequent calls to the
 * function compound the effect. For example, calling
 * `rotate Constants.pi/2. env` once and then calling `rotate Constants.pi/2. env`
 * a second time is the same as a single `rotate Constants.pi env`. All
 * tranformations are reset when `draw` begins again.

 * Technically, `rotate` multiplies the current transformation matrix by a
 * rotation matrix. This function can be further controlled by `pushMatrix`
 * and `popMatrix`.
 */
let rotate: float => Reprocessing_Types.Types.glEnvT => unit;


/** Sets the color used to fill shapes.*/
let fill: Reprocessing_Types.Types.colorT => Reprocessing_Types.Types.glEnvT => unit;


/** Disables filling geometry. If both `noStroke` and `noFill` are called,
   * nothing will be drawn to the screen.
 */
let noFill: Reprocessing_Types.Types.glEnvT => unit;


/** Sets the color used to draw lines and borders around shapes. */
let stroke: Reprocessing_Types.Types.colorT => Reprocessing_Types.Types.glEnvT => unit;


/** Disables drawing the stroke (outline). If both noStroke() and noFill()
   * are called, nothing will be drawn to the screen.
 */
let noStroke: Reprocessing_Types.Types.glEnvT => unit;


/** Sets the width of the stroke used for lines, points, and the border around
   * shapes. All widths are set in units of pixels.
 */
let strokeWeight: int => Reprocessing_Types.Types.glEnvT => unit;


/** The `pushStyle` function saves the current style settings and `popStyle`
   * restores the prior settings. Note that these functions are always used
   * together. They allow you to change the style settings and later return to
   * what you had. When a new style is started with `pushStyle`, it builds on the
   * current style information. The `pushStyle` and `popStyle` functions can be
   * embedded to provide more control.
   *
   * The style information controlled by the following functions are included in
   * the style: fill, stroke, strokeWeight
 */
let pushStyle: Reprocessing_Types.Types.glEnvT => unit;


/** The `pushStyle` function saves the current style settings and
   * `popStyle` restores the prior settings; these functions are always used
   * together. They allow you to change the style settings and later return to
   * what you had. When a new style is started with `pushStyle`, it builds on the
   * current style information. The `pushStyle` and `popStyle` functions can be
   * embedded to provide more control.
   *
   * The style information controlled by the following functions are included in
   * the style: fill, stroke, strokeWeight
 */
let popStyle: Reprocessing_Types.Types.glEnvT => unit;


/** Loads an image and returns a handle to it. This will lazily load and
   * attempting to draw an image that has not finished loading will result
   * in nothing being drawn.
   * In general, all images should be loaded in `setup` to preload them at
   * the start of the program.
 */
let loadImage:
  filename::string => Reprocessing_Types.Types.glEnvT => Reprocessing_Types.Types.imageT;


/** The `image` function draws an image to the display window.
   * The image should be loaded using the `loadImage` function.
   * The image is displayed at its original size.
 */
let image:
  Reprocessing_Types.Types.imageT =>
  pos::(int, int) =>
  Reprocessing_Types.Types.glEnvT =>
  unit;

/* let clear: Reprocessing_Types.Types.glEnvT => unit; */

/** Draws a rectangle to the screen. A rectangle is a four-sided shape with
   * every angle at ninety degrees.
 */
let rectf:
  pos::(float, float) =>
  width::float =>
  height::float =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/** Draws a rectangle to the screen. A rectangle is a four-sided shape with
   * every angle at ninety degrees.
   *
   * This is the same as `rectf`, but converts all its integer arguments to floats
   * as a convenience.
 */
let rect:
  pos::(int, int) =>
  width::int =>
  height::int =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/** Draws a line (a direct path between two points) to the screen.
   * To color a line, use the `stroke` function. A line cannot be filled,
   * therefore the `fill` function will not affect the color of a line. Lines are
   * drawn with a width of one pixel by default, but this can be changed with
   * the `strokeWeight` function.
 */
let linef:
  p1::(float, float) => p2::(float, float) => Reprocessing_Types.Types.glEnvT => unit;


/** Draws a line (a direct path between two points) to the screen.
   * To color a line, use the `stroke` function. A line cannot be filled,
   * therefore the `fill` function will not affect the color of a line. Lines are
   * drawn with a width of one pixel by default, but this can be changed with
   * the `strokeWeight` function.
   *
   * This is the same as `linef`, but converts all its integer arguments to floats
   * as a convenience.
 */
let line:
  p1::(int, int) => p2::(int, int) => Reprocessing_Types.Types.glEnvT => unit;


/** Draws an ellipse (oval) to the screen. An ellipse with equal width and
   * height is a circle.
 */
let ellipsef:
  center::(float, float) =>
  radx::float =>
  rady::float =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/** Draws an ellipse (oval) to the screen. An ellipse with equal width and
   * height is a circle.
   *
   * This is the same as `ellipsef`, but converts all its integer arguments to
   * floats as a convenience.
 */
let ellipse:
  center::(int, int) =>
  radx::int =>
  rady::int =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/**  A quad is a quadrilateral, a four sided polygon. It is similar to a
   * rectangle, but the angles between its edges are not constrained to ninety
   * degrees. The parameter p1 sets the first vertex and the subsequest points
   * should proceed clockwise or counter-clockwise around the defined shape.
 */
let quadf:
  p1::(float, float) =>
  p2::(float, float) =>
  p3::(float, float) =>
  p4::(float, float) =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/**  A quad is a quadrilateral, a four sided polygon. It is similar to a
   * rectangle, but the angles between its edges are not constrained to ninety
   * degrees. The parameter p1 sets the first vertex and the subsequest points
   * should proceed clockwise or counter-clockwise around the defined shape.
   *
   * This is the same as `quadf`, but converts all its integer arguments to
   * floats as a convenience.
 */
let quad:
  p1::(int, int) =>
  p2::(int, int) =>
  p3::(int, int) =>
  p4::(int, int) =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/** Adds a single point with a radius defined by strokeWeight */
let pixelf:
  pos::(float, float) =>
  color::Reprocessing_Types.Types.colorT =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/** Adds a single point with a radius defined by strokeWeight
   *
   * This is the same as `pixelf`, but converts all its integer arguments to
   * floats as a convenience.
 */
let pixel:
  pos::(int, int) =>
  color::Reprocessing_Types.Types.colorT =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/** A triangle is a plane created by connecting three points. */
let trianglef:
  p1::(float, float) =>
  p2::(float, float) =>
  p3::(float, float) =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/** A triangle is a plane created by connecting three points.
   *
   * This is the same as `trianglef`, but converts all its integer arguments to
   * floats as a convenience.
 */
let triangle:
  p1::(int, int) =>
  p2::(int, int) =>
  p3::(int, int) =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/** Draws a Bezier curve on the screen. These curves are defined by a
   * series of anchor and control points. The parameter p1 specifies the
   * first anchor point and the last parameter specifies the other anchor
   * point. The middle parameters p2 and p3 specify the control points
   * which define the shape of the curve. Bezier curves were developed
   * by French engineer Pierre Bezier.
 */
let bezier:
  p1::(float, float) =>
  p2::(float, float) =>
  p3::(float, float) =>
  p4::(float, float) =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/** Draws an arc to the screen. Arcs are drawn along the outer edge of an
   * ellipse defined by the center, radx, and rady parameters. Use the
   * start and stop parameters to specify the angles (in radians) at which
   * to draw the arc. isPie defines whether or not lines should be drawn to
   * the center at the start and stop points of the arc rather than simply
   * connecting the points.  If isOpen is true, no line will be drawn other
   * than the arc between start and stop.
 */
let arcf:
  center::(float, float) =>
  radx::float =>
  rady::float =>
  start::float =>
  stop::float =>
  isOpen::bool =>
  isPie::bool =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/** Draws an arc to the screen. Arcs are drawn along the outer edge of an
   * ellipse defined by the center, radx, and rady parameters. Use the
   * start and stop parameters to specify the angles (in radians) at which
   * to draw the arc. isPie defines whether or not lines should be drawn to
   * the center at the start and stop points of the arc rather than simply
   * connecting the points.  If isOpen is true, no line will be drawn other
   * than the arc between start and stop.
   *
   * This is the same as `arcf`, but converts all its integer arguments to
   * floats as a convenience.
 */
let arc:
  center::(int, int) =>
  radx::int =>
  rady::int =>
  start::float =>
  stop::float =>
  isOpen::bool =>
  isPie::bool =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/** Loads a font and returns a handle to it. This will lazily load and
   * attempting to draw an font that has not finished loading will result
   * in nothing being drawn.
   * In general, all fonts should be loaded in `setup` to preload them at
   * the start of the program.
 */
let loadFont:
  filename::string => Reprocessing_Types.Types.glEnvT => Reprocessing_Font.fontT;


/** Draws text to the screen.
   * The font should be loaded using the `loadFont` function.
 */
let text:
  font::Reprocessing_Font.fontT =>
  body::string =>
  pos::(int, int) =>
  Reprocessing_Types.Types.glEnvT =>
  unit;


/** The `background` function sets the color used for the background of the
   * Processing window. The default background is black. This function is
   * typically used within `draw` to clear the display window at the beginning of
   * each frame, but it can be used inside `setup` to set the background on the
   * first frame of animation or if the backgound need only be set once.
 */
let background:
  Reprocessing_Types.Types.colorT => Reprocessing_Types.Types.glEnvT => unit;
