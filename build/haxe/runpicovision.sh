#! /bin/sh -e

./build/haxe/prepmicropython.sh

. build/haxe/.env/bin/activate
if ! command -v mpy-cross-v6.1 >/dev/null || ! command -v mpremote >/dev/null; then
  printf "Required binaries are missing, please run setuppicovision.sh to acquire them\n"
  exit 1
fi

mpy-cross-v6.1 out/embeddedgamefixed.py -o out/embeddedgame.mpy &
mpy-cross-v6.1 frontends/picovisionhelpers.py -o out/nativehelpers.mpy

mpremote cp out/embeddedgame.mpy out/nativehelpers.mpy :
mpremote exec "import embeddedgame"
