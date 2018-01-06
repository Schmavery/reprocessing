open Reasongl;

module Internal = Reprocessing_Internal;

open Reprocessing_Common;

module Font = {
  module IntMap =
    Map.Make(
      {
        type t = int;
        let compare = compare;
      }
    );
  module IntPairMap =
    Map.Make(
      {
        type t = (int, int);
        let compare = ((a1, a2), (b1, b2)) => {
          let first = compare(a1, b1);
          if (first != 0) {
            first
          } else {
            compare(a2, b2)
          }
        };
      }
    );
  type charT = {
    x: int,
    y: int,
    width: int,
    height: int,
    xoffset: int,
    yoffset: int,
    xadvance: int
  };
  type internalType = {
    chars: IntMap.t(charT),
    kerning: IntPairMap.t(int),
    image: imageT
  };
  type t = ref(option(internalType));
  let rec parse_num = (stream: Stream.t, acc) : (Stream.t, int) =>
    switch (Stream.peekch(stream)) {
    | Some('-' as c)
    | Some('0'..'9' as c) => parse_num(Stream.popch(stream), append_char(acc, c))
    | _ =>
      try (stream, int_of_string(acc)) {
      | _ => failwith("Could not parse number [" ++ (acc ++ "]."))
      }
    };
  let parse_num = (stream) => parse_num(stream, "");
  let rec parse_string = (stream: Stream.t, acc: string) : (Stream.t, string) =>
    switch (Stream.peekch(stream)) {
    | Some('"') => (Stream.popch(stream), acc)
    | Some(c) => parse_string(Stream.popch(stream), append_char(acc, c))
    | None => failwith("Unterminated string.")
    };
  let parse_string = (stream) => parse_string(stream, "");
  let rec pop_line = (stream) =>
    switch (Stream.peekch(stream)) {
    | Some('\n') => Stream.popch(stream)
    | Some(_) => pop_line(Stream.popch(stream))
    | None => failwith("could not pop line")
    };
  let rec parse_char_fmt = (stream, num, map) =>
    if (num <= 0) {
      (stream, map)
    } else {
      let stream = Stream.match(stream, "char id=");
      let (stream, char_id) = parse_num(stream);
      let stream = Stream.match(Stream.skipWhite(stream), "x=");
      let (stream, x) = parse_num(stream);
      let stream = Stream.match(Stream.skipWhite(stream), "y=");
      let (stream, y) = parse_num(stream);
      let stream = Stream.match(Stream.skipWhite(stream), "width=");
      let (stream, width) = parse_num(stream);
      let stream = Stream.match(Stream.skipWhite(stream), "height=");
      let (stream, height) = parse_num(stream);
      let stream = Stream.match(Stream.skipWhite(stream), "xoffset=");
      let (stream, xoffset) = parse_num(stream);
      let stream = Stream.match(Stream.skipWhite(stream), "yoffset=");
      let (stream, yoffset) = parse_num(stream);
      let stream = Stream.match(Stream.skipWhite(stream), "xadvance=");
      let (stream, xadvance) = parse_num(stream);
      let stream = pop_line(stream);
      let new_map = IntMap.add(char_id, {x, y, width, height, xoffset, yoffset, xadvance}, map);
      parse_char_fmt(stream, num - 1, new_map)
    };
  let rec parse_kern_fmt = (stream, num, map) =>
    if (num == 0) {
      (stream, map)
    } else {
      let stream = Stream.match(stream, "kerning first=");
      let (stream, first) = parse_num(stream);
      let stream = Stream.match(stream, " second=");
      let (stream, second) = parse_num(stream);
      let stream = Stream.match(stream, " amount=");
      let (stream, amount) = parse_num(stream);
      let stream = pop_line(stream);
      let new_map = IntPairMap.add((first, second), amount, map);
      parse_kern_fmt(stream, num - 1, new_map)
    };
  let replaceFilename = (path, filename) => {
    let splitStr = Reprocessing_Common.split(path, ~sep='/');
    let revLst = List.rev(splitStr);
    let newRevLst =
      switch revLst {
      | [_, ...tl] => [filename, ...tl]
      | [] => []
      };
    let newLst = List.rev(newRevLst);
    String.concat("/", newLst)
  };
  let parseFontFormat = (env, path, isPixel) => {
    let ret = ref(None);
    Gl.File.readFile(
      ~context=env.gl,
      ~filename=path,
      ~cb=
        (str) => {
          let stream = Stream.create(str ++ "\n");
          let stream = stream |> pop_line |> pop_line;
          let stream = Stream.match(stream, "page id=0 file=\"");
          let (stream, filename) = parse_string(stream);
          let stream = pop_line(stream);
          let stream = Stream.match(stream, "chars count=");
          let (stream, num_chars) = parse_num(stream);
          let stream = pop_line(stream);
          let (stream, char_map) = parse_char_fmt(stream, num_chars, IntMap.empty);
          let stream = Stream.match(stream, "kernings count=");
          let (stream, num_kerns) = parse_num(stream);
          let stream = pop_line(stream);
          let (_, kern_map) = parse_kern_fmt(stream, num_kerns, IntPairMap.empty);
          let img_filename = replaceFilename(path, filename);
          ret :=
            Some({
              chars: char_map,
              kerning: kern_map,
              image: Internal.loadImage(env, img_filename, isPixel)
            })
        }
    );
    ret
  };
  let getChar = (fnt, ch) => {
    let code = Char.code(ch);
    try (IntMap.find(code, fnt.chars)) {
    | _ => failwith("Could not find character " ++ (string_of_int(code) ++ " in font."))
    }
  };
  let drawChar = (env: glEnv, fnt, image, ch: char, last: option(char), x, y) => {
    let c = getChar(fnt, ch);
    let kernAmount =
      switch last {
      | Some(lastCh) =>
        let first = Char.code(lastCh);
        let second = Char.code(ch);
        try (IntPairMap.find((first, second), fnt.kerning)) {
        | _ => 0
        }
      | None => 0
      };
    switch image {
    | Some(img) =>
      Internal.drawImageWithMatrix(
        img,
        ~x=x + c.xoffset + kernAmount,
        ~y=y + c.yoffset,
        ~width=c.width,
        ~height=c.height,
        ~subx=c.x,
        ~suby=c.y,
        ~subw=c.width,
        ~subh=c.height,
        env
      );
      c.xadvance + kernAmount
    | None => c.xadvance + kernAmount
    }
  };
  let drawString = (env: glEnv, fnt, str: string, x, y) =>
    switch fnt^ {
    | None => ()
    | Some(fnt) =>
      switch fnt.image^ {
      | Some(img) =>
        let offset = ref(x);
        let lastChar = ref(None);
        String.iter(
          (c) => {
            let advance = drawChar(env, fnt, Some(img), c, lastChar^, offset^, y);
            offset := offset^ + advance;
            lastChar := Some(c)
          },
          str
        )
      | None => print_endline("loading font.")
      }
    };
  let calcStringWidth = (env, fnt, str: string) =>
    switch fnt^ {
    | None => 0
    | Some(fnt) =>
      let offset = ref(0);
      let lastChar = ref(None);
      String.iter(
        (c) => {
          offset := offset^ + drawChar(env, fnt, None, c, lastChar^, offset^, 0);
          lastChar := Some(c)
        },
        str
      );
      offset^
    };
};

type fontT = ref(option(Font.internalType));
