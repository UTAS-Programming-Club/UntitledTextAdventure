#! /bin/sh -e

./build/haxe/prepmicropython.sh

. build/haxe/.env/bin/activate
if ! command -v mpy-cross-v6.3 >/dev/null || ! command -v mpremote >/dev/null; then
  printf "Required binaries are missing, please run setupsquixl.sh to acquire them\n"
  exit 1
fi

mpy-cross-v6.3 out/embeddedgamefixed.py -o out/embeddedgame.mpy &
mpy-cross-v6.3 frontends/squixlhelpers.py -o out/nativehelpers.mpy

if [ $# -ne 1 ] || [ "$1" != -q ]; then
  mpremote reset | true
  sleep 1
  until [ -c /dev/ttyACM* ]; do
  sleep 0.5
  done
fi

mpremote cp out/embeddedgame.mpy out/nativehelpers.mpy :
mpremote exec "import embeddedgame"
