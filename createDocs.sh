set -e

mv src/Reprocessing_ClientWrapper.re ./tmpClientWrapper
mv README.md tmpREADME.md
cp index.md README.md

sed -i "" -e 's/Reprocessing_ClientWrapper.init/Reasongl.Gl.Window.init/' src/Reprocessing.re

./node_modules/bs-platform/lib/bsb.exe -clean-world -make-world -backend js

ls src

./node_modules/docre/lib/bs/native/main.native

sed -i "" -e 's/Reasongl.Gl.Window.init/Reprocessing_ClientWrapper.init/' src/Reprocessing.re

mv tmpREADME.md README.md
mv ./tmpClientWrapper src/Reprocessing_ClientWrapper.re
