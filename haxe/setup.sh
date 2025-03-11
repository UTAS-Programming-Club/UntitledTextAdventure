#!/bin/sh -e

# TODO: Support using an existing copy of haxe
# TODO: Support aarch64
# TODO: Support macOS
# TODO: Support Windows

haxe_version=4.3.6
#neko_version=2.4.0

NEED_SUBSHELL=false

# Parameters:
# 1. Informative name
# 2. url
# 3. destination path
download_file() {
  echo "Downloading $1"
  if command -v wget > /dev/null 2>&1; then
    wget --show-progress --progress=bar:force:noscroll -qO "$3" "$2"
  elif command -v curl > /dev/null 2>&1; then
    curl -Lf "$2" -o "$3"
  else
    echo "Unable to download required file: $2"
    exit 1
  fi
}

# Parameters:
# 1. Informative name
# 2. archive path
# 3. destination path
extract_tar_gz() {
  echo "Extracting $1"
  mkdir -p "$3"
  if command -v tar > /dev/null 2>&1; then
    tar -xf "$2" -C "$3" --strip-components=1
  elif command -v bsdtar > /dev/null 2>&1; then
    bsdtar -xf "$2" -C "$3" --strip-components=1
  else
    echo "Unable to extract required file: $2"
    exit 1
  fi
}

if [ "$#" -eq 1 ] && [ "$1" = "-f" ]; then
  rm -rf .haxelib hashlink haxe neko out tmp
fi

if [ ! -d haxe ]; then
  download_file Haxe https://github.com/HaxeFoundation/haxe/releases/download/$haxe_version/haxe-$haxe_version-linux64.tar.gz haxe.tar.gz
  extract_tar_gz Haxe haxe.tar.gz haxe
  rm haxe.tar.gz
  printf "\n"
  NEED_SUBSHELL=true
fi

if [ ! -d neko ]; then
  download_file Neko https://github.com/HaxeFoundation/neko/releases/download/v2-4-0/neko-2.4.0-linux64.tar.gz neko.tar.gz
  extract_tar_gz Neko neko.tar.gz neko
  rm neko.tar.gz
  printf "\n"
fi

if [ ! -d .haxelib ]; then
  echo "Downloading haxelib repo"
  LD_LIBRARY_PATH=$PWD/neko:$LD_LIBRARY_PATH haxe/haxelib newrepo
  LD_LIBRARY_PATH=$PWD/neko:$LD_LIBRARY_PATH haxe/haxelib git hashlink https://github.com/HaxeFoundation/hashlink.git master other/haxelib/
  printf "\n"
fi

if ! command -v haxe > /dev/null 2>&1; then
  PATH=$PWD/haxe:$PATH
  NEED_SUBSHELL=true
fi

if ! command -v neko > /dev/null 2>&1; then
  PATH=$PWD/neko:$PATH
  LD_LIBRARY_PATH=$PWD/neko:$LD_LIBRARY_PATH
  NEED_SUBSHELL=true
fi

if ! command -v hl > /dev/null 2>&1; then
  PATH=$PWD/hashlink:$PATH
  LD_LIBRARY_PATH=$PWD/hashlink:$LD_LIBRARY_PATH
  NEED_SUBSHELL=true
fi

if [ $NEED_SUBSHELL = true ]; then
  echo "Launching subshell"
  LD_LIBRARY_PATH=$LD_LIBRARY_PATH $SHELL
fi
