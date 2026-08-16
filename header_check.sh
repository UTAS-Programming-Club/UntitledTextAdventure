#!/usr/bin/env sh
set -eu

IWYU="${IWYU:-../C/include-what-you-use/build/bin/include-what-you-use}"

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
-pedantic -I src/ \
-Xiwyu --update_comments -Xiwyu --max_line_length=250"

test_dsl_src()
(
  # shellcheck disable=SC2086
  "$IWYU" -std=c23 $CFLAGS $UTA_C_FLAGS "$1"
)

test_game_src()
(
  # shellcheck disable=SC2086
  "$IWYU" --std=c99 -I . $CFLAGS $UTA_C_FLAGS "$1"
)

test_dsl_src src/dsl/dsl.c
read -r IGNORE

test_game_src src/backend.c
read -r IGNORE
test_game_src src/coregame2.c
# shellcheck disable=SC2034
read -r IGNORE
test_game_src src/frontend.c
