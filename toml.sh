FILES=src/*
for f in $FILES
do
  echo "Processing $f"
  IFS='.' read -r -a array <<< "$f"
  ./node_modules/reason/refmt_impl.native -parse re -print ml $f > "${array[0]}.ml"
done

./node_modules/reason/refmt_impl.native -parse re -print ml node_modules/reglinterface/src/constants.re > constants.ml
./node_modules/reason/refmt_impl.native -parse re -print ml node_modules/reglinterface/src/gl.re > reglinterface.ml

echo "module Webgl = Webgl" > reglweb.ml

./node_modules/reason/refmt_impl.native -parse re -print ml node_modules/reglweb/src/webgl.re > webgl.ml

./node_modules/bs-platform/bin/bspack.exe -I src -I . -bs-main Reprocessing -o test.ml

GL_BACKEND=web ./node_modules/ocaml/bin/ocamlc -dsource -ppx ./node_modules/matchenv/ppx_matchenv.exe test.ml

./node_modules/bs-platform/bin/bspack.exe -I src -I . -bs-main Test -o test.ml
