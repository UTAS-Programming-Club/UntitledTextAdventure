#!/usr/bin/env sh
set -eu

./scripts/setup.sh

cmake -S src/frontends/picocalc/ -B out/picocalc/
cmake --build out/picocalc/
