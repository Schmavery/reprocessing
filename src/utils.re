open Common;

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
  let color ::r ::g ::b :colorT => {r, g, b};
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
  let rec split stream sep accstr acc =>
    switch (Stream.peekch stream) {
    | Some c when c == sep => split (Stream.popch stream) sep "" [accstr, ...acc]
    | Some c => split (Stream.popch stream) sep (append_char accstr c) acc
    | None => List.rev [accstr, ...acc]
    };
  let split str sep => split (Stream.create str) sep "" [];
};
