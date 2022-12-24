export LD_LIBRARY_PATH=$(dirname "$0")/lib:$LD_LIBRARY_PATH
find ldd lib/ -exec printf '------\n{}\n' \; -exec sh ldd {} \;
