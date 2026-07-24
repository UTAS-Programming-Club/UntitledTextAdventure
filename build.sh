#!/usr/bin/env sh
set -e

CORES=$(($(nproc) + 1))

if [ ! -f third_party/re2c/re2c ]; then
    (cd third_party/re2c/ && ./configure --disable-shared)
    make -C ./third_party/re2c/ CFLAGS="-D _DEFAULT_SOURCE" -j"$CORES"
fi

mkdir -p gen/
./third_party/re2c/re2c src/dsl.c -o gen/dsl.gen.c

mkdir -p bin/
clang-22 -O3 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
-Werror=format-security \
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 \
-D_GLIBCXX_ASSERTIONS \
-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST \
-fstrict-flex-arrays=3 \
-fstack-clash-protection -fstack-protector-strong \
-Wl,-z,nodlopen -Wl,-z,noexecstack \
-Wl,-z,relro -Wl,-z,now \
-Wl,--as-needed -Wl,--no-copy-dt-needed-entries \
-fcf-protection=full \
-fno-delete-null-pointer-checks -fno-strict-overflow -fno-strict-aliasing -ftrivial-auto-var-init=zero \
-Werror=implicit -Werror=incompatible-pointer-types -Werror=int-conversion \
$UTA_C_FLAGS -pedantic \
-std=c23 gen/dsl.gen.c -o bin/utatest2026-dsl

./bin/utatest2026-dsl src/ext1.uta gen/ext1.h gen/ext1.c

clang-22 -O3 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
-Werror=format-security \
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 \
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
$UTA_C_FLAGS -pedantic -I . -I src/ \
-std=c23 src/frontend.c src/backend.c src/coregame.c gen/ext1.c -o bin/utatest2026
