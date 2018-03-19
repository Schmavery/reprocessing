---
id: doc5
title: Utils
---

### `color: (~r: int, ~g: int, ~b: int, ~a: int) => colorT`

Creates colors for storing in variables of the color datatype.

Components should be in the range 0 to 255 (or 0x00 to 0xFF).

---

### `colorf: (~r: float, ~g: float, ~b: float, ~a: float) => colorT`

Creates colors for storing in variables of the color datatype.

Components should be in the range 0.0 to 1.0.

---

### `round: float => float`

Calculates the integer closest to the input. For example,
`round(133.8)` returns the value 134.

---

### `sq: int => int`

Squares a number (multiplies a number by itself). The result is always a
positive number, as multiplying two negative numbers always yields a
positive result.

---

### `pow: (~base: int, ~exp: int) => int`

Facilitates exponential expressions. The pow() function is an efficient
way of multiplying numbers by themselves (or their reciprocals) in large
quantities.

---

### `constrain: (~amt: 'a, ~low: 'a, ~high: 'a) => 'a`

Constrains a value to not exceed a maximum and minimum value.

---

### `remapf: (~value: float, ~low1: float, ~high1: float, ~low2: float, ~high2: float) => float`

Re-maps a number from one range to another.
i.e. `remapf(~value=5., ~start1=0., ~stop1=10., ~start2=10., ~stop2=20.)`
would give 15.
Useful for scaling values.

---

### `remap: (~value: int, ~low1: int, ~high1: int, ~low2: int, ~high2: int) => int`

Re-maps a number from one range to another.
i.e. `remapf(~value=5., ~start1=0., ~stop1=10., ~start2=10., ~stop2=20.`
would give 15.
Useful for scaling values.

This is the same as `remapf`, but converts all its integer arguments to floats
as a convenience.

---

### `norm: (~value: float, ~low: float, ~high: float) => float`

Normalizes a number from another range into a value between 0 and 1.
Identical to `remap(~value, ~low=0.0, ~high=1.0)`

---

### `randomf: (~min: float, ~max: float) => float`

Generates random numbers. Each time the `randomf` function is called, it
returns an unexpected value within the specified range. The top number is
not included.

---

### `random: (~min: int, ~max: int) => int`

Generates random numbers. Each time the `random` function is called, it
returns an unexpected value within the specified range. The top number is
not included.

This is the same as `randomf`, but converts all its integer arguments to floats
as a convenience.

---

### `randomSeed: int => unit`

Sets the seed value for `random` and `randomf`. By default, `random`
produces different results each time the program is run. Set the
seed parameter to a constant to return the same pseudo-random
numbers each time the software is run.
This is equivalent to setting Random.init in ocaml/reason.

---

### `randomGaussian: unit => float`

Returns a float from a random series of numbers having a mean of 0 and
standard deviation of 1. Each time the `randomGaussian` function is called,
it returns a number fitting a Gaussian, or normal, distribution. There is
theoretically no minimum or maximum value that `randomGaussian might
return. Rather, there is just a very low probability that values far from
the mean will be returned; and a higher probability that numbers near the
mean will be returned.

---

### `lerpf: (~low: float, ~high: float, ~value: float) => float`

Calculates a number between two numbers at a specific increment. The
amt parameter is the amount to interpolate between the two values where 0.0
equal to the `low` point, 0.1 is very near the `low` point, 0.5 is half-way
in between, etc. The lerp function is convenient for creating motion along a
straight path and for drawing dotted lines.

---

### `lerp: (~low: int, ~high: int, ~value: float) => int`

Calculates a number between two numbers at a specific increment. The
amt parameter is the amount to interpolate between the two values where 0.0
equal to the `low` point, 0.1 is very near the `low` point, 0.5 is half-way
in between, etc. The lerp function is convenient for creating motion along a
straight path and for drawing dotted lines.

This is the same as `lerpf`, but converts all its integer arguments to floats
as a convenience.

---

### `lerpColor: (~low: colorT, ~high: colorT, ~value: float) => colorT`

---

### `distf: (~p1: (float, float), ~p2: (float, float)) => float`

Calculates the distance between two points.

---

### `dist: (~p1: (int, int), ~p2: (int, int)) => float`

Calculates the distance between two points.

This is the same as `distf`, but converts all its integer arguments to floats
as a convenience.

---

### `magf: ((float, float)) => float`

Calculates the magnitude (or length) of a vector. A vector is a direction
in space commonly used in computer graphics and linear algebra. Because it
has no "start" position, the magnitude of a vector can be thought of as the
distance from the coordinate 0,0 to its x,y value. Therefore, `magf` is a
shortcut for writing `distf((0., 0.), (x, y))`.

---

### `mag: ((int, int)) => float`

Calculates the magnitude (or length) of a vector. A vector is a direction
in space commonly used in computer graphics and linear algebra. Because it
has no "start" position, the magnitude of a vector can be thought of as the
distance from the coordinate 0,0 to its x,y value. Therefore, `mag` is a
shortcut for writing `dist((0, 0), (x, y))`.

This is the same as `magf`, but converts all its integer arguments to floats
as a convenience.

---

### `degrees: float => float`

Converts a radian measurement to its corresponding value in degrees.
Radians and degrees are two ways of measuring the same thing. There are 360
degrees in a circle and 2*PI radians in a circle. For example, 90° = PI/2 =
1.5707964. All trigonometric functions require their parameters to be
specified in radians.

---

### `radians: float => float`

Converts a degree measurement to its corresponding value in radians.
Radians and degrees are two ways of measuring the same thing. There are 360
degrees in a circle and 2*PI radians in a circle. For example, 90° = PI/2 =
1.5707964. All trigonometric functions require their parameters to be
specified in radians.

---

### `noise: (float, float, float) => float`

Returns the Perlin noise value at specified coordinates. Perlin noise is a
random sequence generator producing a more natural, harmonic succession of
numbers than that of the standard random() function. It was developed by Ken
Perlin in the 1980s and has been used in graphical applications to generate
procedural textures, shapes, terrains, and other seemingly organic forms.

In contrast to the random() function, Perlin noise is defined in an infinite
n-dimensional space, in which each pair of coordinates corresponds to a
fixed semi-random value (fixed only for the lifespan of the program). The
resulting value will always be between 0.0 and 1.0. The noise value can be
animated by moving through the noise space.

The actual noise structure is similar to that of an audio signal, in respect
to the function's use of frequencies. Similar to the concept of harmonics in
physics, Perlin noise is computed over several octaves which are added together
for the final result.

Another way to adjust the character of the resulting sequence is the scale of
the input coordinates. As the function works within an infinite space, the
value of the coordinates doesn't matter as such; only the distance between
successive coordinates is important (such as when using noise() within a loop).
As a general rule, the smaller the difference between coordinates, the smoother
the resulting noise sequence. Steps of 0.005-0.03 work best for most
applications, but this will differ depending on use.

---

### `noiseSeed: int => unit`

Sets the seed value for `noise`.  This will also affect the
seed value for `random` and `randomf`.

---

### `split: (string, ~sep: char) => list(string)`

The `split` function breaks a string into pieces using a character
as the delimiter. The `sep` parameter specifies the character
that mark the boundaries between each piece. A list is returned
that contains each of the pieces.

---

### `intersectRectCircle: ( ~rectPos: (float, float), ~rectW: float, ~rectH: float, ~circlePos: (float, float), ~circleRad: float) => bool`

Determines if there is an intersection between a rectangle and a circle.
`rectPos` refers to the top left of the rect and `circlePos` to the center of
the circle. `rectW` and `rectH` are the width and height of the rectangle and
`circleRad` is the radius of the circle.
Returns true if the two shapes overlap.

---

### `intersectRectRect: ( ~rect1Pos: (float, float), ~rect1W: float, ~rect1H: float, ~rect2Pos: (float, float), ~rect2W: float, ~rect2H: float) => bool`

Determines if there is an intersection between two axis-aligned rectangles.
`rect1Pos` and `rect2Pos` refer to the top left of the first rectangles.
`rect1W` and `rect1H` are the width and height of the first rectangle and 
correspondingly for the second.
Returns true if the two shapes overlap.
