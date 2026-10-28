#!/usr/bin/env sh
set -eu

./scripts/setup.sh

export DEVKITPRO="$PWD/third_party/dkp"
export DEVKITARM="$DEVKITPRO/devkitARM"

if [ ! -d "$DEVKITPRO/" ]; then
  patch third_party/dkp-buildscripts/ src/frontends/ds/dkp-buildscripts.diff
  (
    cd third_party/dkp-buildscripts/
    ./build-devkit.sh
  )

  cp -r third_party/catnip/cmake/ third_party/dkp/
  cp third_party/pacman-packages/cmake/common-utils/dkp* \
     third_party/pacman-packages/cmake/devkitarm/*.cmake \
     third_party/pacman-packages/cmake/devkitarm/dkp-arm-mode-wrapper \
     third_party/pacman-packages/cmake/gba/GBA.cmake \
     third_party/pacman-packages/cmake/nds/NDS.cmake \
     third_party/dkp/cmake/
  mkdir -p third_party/dkp/cmake/Platform/ third_party/dkp/portlibs/nds/bin/
  cp third_party/pacman-packages/cmake/common-utils/Generic-dkP.cmake third_party/pacman-packages/cmake/nds/NintendoDS.cmake third_party/dkp/cmake/Platform/
  cp third_party/pacman-packages/nds/pkg-config/pkg-config.in third_party/dkp/portlibs/nds/bin/arm-none-eabi-pkg-config

  (
    cd third_party/calico/
    bash ../catnip/catnip -v install
  )

  (
    cd third_party/general-tools/
    ./autogen.sh
    autoreconf
    ./configure
    make
  )

  (
    cd third_party/grit/
    ./autogen.sh
    autoreconf
    ./configure
    make
  )

  PATH="$PWD/third_party/general-tools:$PWD/third_party/grit:$PATH" make -C third_party/libnds/ install

  (
    cd third_party/ndstool/
    ./autogen.sh
    autoreconf
    ./configure
    make
  )

  (
    cd third_party/maxmod/
    bash ../catnip/catnip -v install
  )

  (
    cd third_party/default-arm7/
    bash ../catnip/catnip -v install
  )
fi

PATH="$PWD/third_party/ndstool:$PATH" make -C src/frontends/ds/
