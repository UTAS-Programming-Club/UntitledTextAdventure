#!/usr/bin/env sh
set -eu

./scripts/setup.sh

if [ ! -f src/frontends/ps2/schrift.c ]; then
  cp third_party/libschrift/schrift.c src/frontends/ps2/schrift.c
  patch src/frontends/ps2/schrift.c src/frontends/ps2/libschrift-ps2.diff
fi

cmake -S src/frontends/ps2/ -B out/ps2/ "-DCMAKE_TOOLCHAIN_FILE=${PS2SDK}/ps2dev.cmake" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=${PS2SDK}/ports"
cmake --build out/ps2/
