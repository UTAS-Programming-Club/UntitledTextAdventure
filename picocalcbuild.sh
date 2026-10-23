#!/usr/bin/env sh
set -eu

./scripts/setup.sh

cmake -S src/frontends/picocalc/ -B out/picocalc/ -DCMAKE_BUILD_TYPE=Release
cmake --build out/picocalc/
