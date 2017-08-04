# Build with ClientWrapper
./node_modules/.bin/bsb -clean-world
cp ./src/Reprocessing_ClientWrapper.re src/Reprocessing_ClientWrapper.re.bak && cp ./webenv-artifacts/Reprocessing_ClientWrapper.re ./src/Reprocessing_ClientWrapper.re
./node_modules/.bin/bsb -make-world -backend js
mv ./src/Reprocessing_ClientWrapper.re.bak ./src/Reprocessing_ClientWrapper.re

# Copy to js build location
mkdir ./webenv-build
cp ./lib/js/src/* ./webenv-build
cp ./webenv-artifacts/entrypoint.js ./webenv-build

# Build browserified bundle
browserify ./webenv-build/entrypoint.js -o reprocessing-bundle.js

curr_branch=$(git rev-parse --abbrev-ref HEAD)

git checkout gh-pages
vp reprocessing-bundle.js reprocessing.js
git add reprocessing.js
git commit -m "Update reprocessing bundle."
git push

git checkout $curr_branch
