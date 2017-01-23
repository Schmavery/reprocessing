FILES=src/*
for f in $FILES
do
  echo "Processing $f"
  IFS='.' read -r -a array <<< "$f"
  ./node_modules/reason/refmt_impl.native -parse re -print ml $f > "${array[0]}.ml"
done

./node_modules/reason/refmt_impl.native -parse re -print ml node_modules/reglweb/src/webgl.re > src/glloader.ml

./node_modules/reason/refmt_impl.native -parse re -print ml node_modules/reglinterface/src/constants.re > constants.ml
./node_modules/reason/refmt_impl.native -parse re -print ml node_modules/reglinterface/src/gl.re > gl.ml
echo "module Constants = Constants\n" > reglinterface.ml
echo "module Gl = Gl\n" >> reglinterface.ml

sed -i -e 's/Gl.Window.init/ClientWrapper.init/g' src/reprocessing.ml
rm src/reprocessing.ml\-e

./node_modules/bs-platform/bin/bspack.exe -I src -I . -bs-main Reprocessing -o mainBundle.ml

sed -i -e '1s/^/'"$clientWrapper"'/' mainBundle.ml
