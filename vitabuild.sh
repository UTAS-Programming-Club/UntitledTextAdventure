#!/usr/bin/env sh
set -eu

./scripts/setup.sh

# shellcheck disable=SC2250
export VITASDK="$PWD/third_party/vitasdk/"
# shellcheck disable=SC2250
export PATH="$VITASDK/bin:$PATH"

if [ ! -d third_party/vitasdk/ ]; then
  third_party/vdpm/bootstrap-vitasdk.sh
  yes | vdpm install libvita2d
fi

# don't think I need anything else?
cmake -S src/frontends/vita -B out/vita/ -DCMAKE_BUILD_TYPE=Release
cmake --build out/vita/
