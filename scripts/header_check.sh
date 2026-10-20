#!/usr/bin/env sh
set -eu

IWYU="${IWYU:-../C/include-what-you-use/build/bin/include-what-you-use}"
UTA_C_FLAGS="${UTA_C_FLAGS:-}"

CFLAGS="-O3 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
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
-pedantic -I . -I src/ -I out/gen/ \
-Xiwyu --update_comments -Xiwyu --max_line_length=250"

test_src_c23()
(
	# shellcheck disable=SC2086
	"$IWYU" -std=c23 $CFLAGS ${2:-} $UTA_C_FLAGS "$1"
)

test_src_c99()
(
	# shellcheck disable=SC2086
	"$IWYU" --std=c99 $CFLAGS $UTA_C_FLAGS "$1"
)

test_src_c23 src/dsl/dsl.c
read -r IGNORE
test_src_c99 src/backend.c
read -r IGNORE
test_src_c99 src/coregame2.c
read -r IGNORE
test_src_c23 src/frontends/textfrontend.c
read -r IGNORE
test_src_c23 src/frontends/guifrontend.c "-I third_party/raylib/src/ -I third_party/libschrift/"
# shellcheck disable=SC2034
read -r IGNORE
"$IWYU" src/frontends/ps2/frontend.c -std=c23 -Xiwyu --update_comments -Xiwyu --max_line_length=1000 -I third_party/raylib/src -I . -I src/ -I out/gen/ -I third_party/libschrift -I third_party/ps2dev/gsKit/include -I third_party/ps2dev/ps2sdk/common/include -D _EE -I third_party/ps2dev/ps2sdk/ee/include -I third_party/LoadBMP
