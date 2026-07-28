#!/usr/bin/env sh
set -e

CORES=$(($(nproc) + 1))

if [ ! -f third_party/re2c/re2c ]; then
    (cd third_party/re2c/ && ./configure --disable-shared)
    make -C ./third_party/re2c/ CFLAGS="-D _DEFAULT_SOURCE" -j"$CORES"
fi

mkdir -p gen/
./third_party/re2c/re2c src/dsl/lexer.c -o gen/lexer.gen.c


# Save for release builds, causes clang to ignore printf argument related warnings
# -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3

mkdir -p bin/
clang-22 -O3 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
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
-std=c23 -pedantic -I src \
$UTA_C_FLAGS \
gen/lexer.gen.c src/dsl/parser.c src/dsl/codegen.c src/dsl/dsl.c -o bin/utatest2026-dsl

./bin/utatest2026-dsl src/coregame.uta gen/coregame.h gen/coregame.c Core
./bin/utatest2026-dsl src/ext1.uta gen/ext1.h gen/ext1.c Ext1

clang-22 -O3 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
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
-std=c23 -pedantic -I . -I src/ \
$UTA_C_FLAGS \
src/frontend.c src/backend.c src/coregame2.c gen/coregame.c gen/ext1.c -o bin/utatest2026
