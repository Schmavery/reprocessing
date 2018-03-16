---
id: doc4
title: Draw
---

### `translate: (~x: float, ~y: float, glEnvT) => unit`

Specifies an amount to displace objects within the display window.
The dx parameter specifies left/right translation, the dy parameter
specifies up/down translation.

Transformations are cumulative and apply to everything that happens
after and subsequent calls to the function accumulates the effect.
For example, calling `translate dx::50 dy::0 env` and then
`translate dx::20 dy::0 env` is the same as `translate dx::70 dy::0 env`.
If `translate` is called within `draw`, the transformation is reset
when the loop begins again. This function can be further controlled
by using `pushMatrix` and `popMatrix`.

---

### `rotate: (float, glEnvT) => unit`

Rotates the amount specified by the angle parameter. Angles must be
specified in radians (values from 0 to two_pi), or they can be converted
from degrees to radians with the `radians` function.

The coordinates are always rotated around their relative position to the
origin. Positive numbers rotate objects in a clockwise direction and
negative numbers rotate in the couterclockwise direction. Transformations
apply to everything that happens afterward, and subsequent calls to the
function compound the effect. For example, calling
`rotate Constants.pi/2. env` once and then calling `rotate Constants.pi/2. env`
a second time is the same as a single `rotate Constants.pi env`. All
tranformations are reset when `draw` begins again.

Technically, `rotate` multiplies the current transformation matrix by a
rotation matrix. This function can be further controlled by `pushMatrix`
and `popMatrix`.
