FILES=examples/*
for f in $FILES
do
  echo "Compiling $f"
  IFS='.' read -r -a array <<< "$f"
  if [[ $f =~ \.re ]]; then
    cat mainBundle.ml > "${array[0]}.ml"
    ./node_modules/reason/refmt_impl.native -parse re -print ml "${array[0]}.re" >> "${array[0]}.ml"
    node_modules/bs-platform/bin/bsc.exe "${array[0]}.ml"
    node_modules/.bin/webpack "${array[0]}.js" "${array[0]}Packed.js"
  fi
done
