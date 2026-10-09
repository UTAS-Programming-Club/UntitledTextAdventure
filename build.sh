#!/usr/bin/env sh
set -eu

# Save for release builds, causes clang to ignore printf argument related warnings
# -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3

# TODO: Add -Wtrampolines -fzero-init-padding-bits=all when building with GCC

CC="${CC:-cc}"
CFLAGS="-O3 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
-Werror=format-security \
-D_GLIBCXX_ASSERTIONS \
-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST \
-fstrict-flex-arrays=3 \
-fstack-clash-protection -fstack-protector-strong \
-Wl,-z,nodlopen -Wl,-z,noexecstack \
-Wl,-z,relro -Wl,-z,now \
-Wl,--as-needed -Wl,--no-copy-dt-needed-entries \
-fPIE -pie \
-fcf-protection=full \
-fno-delete-null-pointer-checks -fno-strict-overflow -fno-strict-aliasing -ftrivial-auto-var-init=zero \
-Werror=implicit -Werror=incompatible-pointer-types -Werror=int-conversion \
-pedantic -I src/"
UTA_C_FLAGS="${UTA_C_FLAGS:-}"

mkdir -p bin/
# shellcheck disable=SC2250,SC2086
"$CC" -std=c23 $CFLAGS $UTA_C_FLAGS \
src/dsl/dsl.c -o bin/utatest2026-dsl

mkdir -p gen/
./bin/utatest2026-dsl src/coregame.uta gen/coregame.h gen/coregame.c
./bin/utatest2026-dsl src/ext1.uta gen/ext1.h gen/ext1.c

mkdir -p tmp/
for rlsrc in third_party/raylib/src/*.c; do
  # shellcheck disable=SC2250
  if [ ! -f "$rlsrc".o ]; then
    "$CC" -std=c23 -I . \
     -D_GNU_SOURCE -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_33 \
     -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing \
     -std=c99 -fPIC -O1 -Werror=implicit-function-declaration \
     -D_GLFW_X11 \
     -I third_party/raylib/src/external/glfw/include/ \
    "$rlsrc" -c -o "$rlsrc".o
  fi
done

# shellcheck disable=SC2250,SC2086
"$CC" -std=c23 -I . $CFLAGS $UTA_C_FLAGS \
src/frontend.c src/backend.c src/coregame2.c gen/coregame.c gen/ext1.c -o bin/utatest2026

# shellcheck disable=SC2250,SC2086
"$CC" -std=c23 -I . $CFLAGS $UTA_C_FLAGS \
-I third_party/libschrift/ \
-I third_party/raylib/src/ \
third_party/libschrift/schrift.c \
third_party/raylib/src/*.c.o -lm -lX11 \
src/guifrontend.c src/backend.c src/coregame2.c gen/coregame.c gen/ext1.c -o bin/utatest2026gui
