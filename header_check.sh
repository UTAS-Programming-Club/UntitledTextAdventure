#!/usr/bin/env sh
set -e

test_dsl_src()
(
  ../C/include-what-you-use/build/bin/include-what-you-use -O3 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
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
  -std=c23 -pedantic -I src \
  $UTA_C_FLAGS \
  -Xiwyu --update_comments -Xiwyu --max_line_length=250 \
  $1
)

test_game_src()
(
  ../C/include-what-you-use/build/bin/include-what-you-use -O3 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
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
  --std=c99 -pedantic -I . -I src/ \
  $UTA_C_FLAGS \
  -Xiwyu --update_comments -Xiwyu --max_line_length=250 \
  $1
)

test_dsl_src src/dsl/dsl.c
read IGNORE

test_game_src src/backend.c
read IGNORE
test_game_src src/coregame2.c
read IGNORE
test_game_src src/frontend.c
