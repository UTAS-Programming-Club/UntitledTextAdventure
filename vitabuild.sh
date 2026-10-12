#!/usr/bin/env sh

./scripts/setup.sh

# don't think I need anything else?
cmake -S src/vita -B vitabuild -DCMAKE_BUILD_TYPE=Release
cmake --build vitabuild
