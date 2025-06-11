#! /bin/sh -e

if [ ! -d build/haxe/.env ]; then
  printf "Required binaries are missing, please run setupsquixl.sh to acquire them\n"
  exit 1
fi

. build/haxe/.env/bin/activate

if ! command -v mpy-cross-v6.3 >/dev/null || ! command -v mpremote >/dev/null; then
  printf "Required binaries are missing, please run setupsquixl.sh to acquire them\n"
  exit 1
fi

haxe build/haxe/buildembedded.hxml
sed -e 's/import inspect as python_lib_Inspect//'\
    -e 's/import traceback as python_lib_Traceback//'\
    -e 's/Math\.\([A-Z_]\{1,\}\)/\1/g'\
    -e 's/python_lib_Inspect.isclass(\([^)]\{1,\}\))/isinstance(\1,type)/'\
    -e 's/if (\(self._hx_disable_getattr\)):/if not \1 and name in self.__dict__:return self.__dict__[name]/'\
    -e "s/    \(raise AttributeError('field does not exist')\)/elif True: \1/"\
    out/embeddedgame.py > out/embeddedgamefixed.py

mpy-cross-v6.3 out/embeddedgamefixed.py -o out/embeddedgame.mpy &
mpy-cross-v6.3 frontends/squixlhelpers.py -o out/nativehelpers.mpy &

if [ $# -ne 1 ] || [ "$1" != -q ]; then
  mpremote reset | true
  sleep 1
  until [ -c /dev/ttyACM* ]; do
  sleep 0.5
  done
fi

mpremote cp out/embeddedgame.mpy out/nativehelpers.mpy :
mpremote exec "import embeddedgame"
