#!/usr/bin/env sh
set -eu

./scripts/setup.sh

cmake -S src/ps2/ -B ps2build "-DCMAKE_TOOLCHAIN_FILE=${PS2SDK}/ps2dev.cmake" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=${PS2SDK}/ports"
cmake --build ps2build

# TODO: Replace with lib
cp ps2dev/src/gsKit/examples/font/dejavu.bmp ps2build/
cp ps2dev/src/gsKit/examples/font/dejavu.dat ps2build/
