set -e

# Build with ClientWrapper
./node_modules/.bin/bsb -clean-world
cp ./src/Reprocessing_ClientWrapper.re src/Reprocessing_ClientWrapper.re.bak && cp ./webenv-artifacts/Reprocessing_ClientWrapper.re ./src/Reprocessing_ClientWrapper.re
./node_modules/.bin/bsb -make-world -backend js
mv ./src/Reprocessing_ClientWrapper.re.bak ./src/Reprocessing_ClientWrapper.re

# Copy to js build location
mkdir -p ./webenv-artifacts/webenv-build
mkdir -p ./webenv-artifacts/transfer
cp ./lib/js/src/* ./webenv-artifacts/webenv-build
cp ./webenv-artifacts/entrypoint.js ./webenv-artifacts/webenv-build

# Build browserified bundle
./node_modules/.bin/browserify ./webenv-artifacts/webenv-build/entrypoint.js -o ./webenv-artifacts/transfer/reprocessing-bundle.js

cp ./webenv-artifacts/Reprocessing_Ext.re ./webenv-artifacts/transfer

curr_branch=$(git rev-parse --abbrev-ref HEAD)
git stash

git checkout gh-pages
cp ./webenv-artifacts/transfer/reprocessing-bundle.js reprocessing.js
cp ./webenv-artifacts/transfer/Reprocessing_Ext.re Reprocessing_Ext.re

if [ $1 == "--non-interactive" ]
then
  git add reprocessing.js Reprocessing_Ext.re
  git commit -m "Update reprocessing bundle."
  git push
  git checkout $curr_branch
  git stash pop
  exit 0
fi

after=$(git rev-parse --abbrev-ref HEAD)
echo "Committing update to $after\n"
read -p "Are you sure? " -n 1 -r
echo    # (optional) move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
  git add reprocessing.js Reprocessing_Ext.re
  git commit -m "Update reprocessing bundle."
  git push
fi

git checkout $curr_branch
git stash pop
