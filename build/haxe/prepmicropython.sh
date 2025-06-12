#! /bin/sh -e

if [ ! -d build/haxe/.env ]; then
  printf "Required binaries are missing, please run setupsquixl.sh to acquire them\n"
  exit 1
fi

haxe build/haxe/buildembedded.hxml
# inspect.is* replacements are based on https://github.com/micropython/micropython-lib/blob/master/python-stdlib/inspect/inspect.py
# Licensed under MIT, I think, could be PSFL 2
# https://github.com/micropython/micropython-lib/blob/master/LICENSE is slightly unclear as it says modules are under stated licenses and
# that non module files are under MIT unless otherwise stated but I can't see one for inspect or even python-stdlib so assuming non module.
sed -e 's/import inspect as python_lib_Inspect//'\
    -e 's/import functools as python_lib_Functools//'\
    -e 's/import traceback as python_lib_Traceback//'\
    -e 's/Math\.\([A-Z_]\{1,\}\)/\1/g'\
    -e 's/python_lib_Inspect.isclass(\([^)]\{1,\}\))/isinstance(\1,type)/'\
    -e 's/python_lib_Inspect.isfunction(\([^)]\{1,\}\))/isinstance(\1, type(id))/'\
    -e 's/python_lib_Inspect.ismethod(\([^)]\{1,\}\))/isinstance(\1, type("".find))/'\
    -e 's/if (\(self._hx_disable_getattr\)):/if not \1 and name in self.__dict__:return self.__dict__[name]/'\
    -e "s/    \(raise AttributeError('field does not exist')\)/elif True: \1/"\
    out/embeddedgame.py > out/embeddedgamefixed.py
