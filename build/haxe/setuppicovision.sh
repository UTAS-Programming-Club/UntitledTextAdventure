#!/bin/sh -e

if ! command -v python3 >/dev/null; then
  printf "Python 3 is missing, please acquire before continuing\n"
fi

if ! command -v pip3 >/dev/null; then
  printf "Python's pip is missing, please acquire before continuing\n"
fi

if ! python3 -c "import venv"; then
  printf "Python's venv is missing, please acquire before continuing\n"
fi

python3 -m venv build/haxe/.env
. build/haxe/.env/bin/activate

pip3 install mpy-cross-v6.1 mpremote

# TODO: Install MicroPython if not already
