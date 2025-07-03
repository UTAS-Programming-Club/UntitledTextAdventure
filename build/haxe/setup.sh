#!/bin/sh -e

# TODO: Support using an existing copy of haxe
# TODO: Support aarch64
# TODO: Support macOS
# TODO: Support Windows
# TODO: Update to Haxe 4.3.7
# TODO: Update to Neko 2.4.1
# TODO: Update to hxcpp 4.3.90


haxe_version=4.3.6
neko_version=2.4.0
neko_version_2=2-4-0
hxlib_version=4.3.2
hxlib_version_2=4,3,2

base_dir="$PWD/build/haxe"
haxe_dir="$base_dir/haxe"
neko_dir="$base_dir/neko"
haxelib_dir="$base_dir/.haxelib"

need_haxe=false
need_neko=false
need_subshell=false

# Parameters:
# 1. Informative name
# 2. url
# 3. destination path
download_file() {
  printf "Downloading %s\n" "$1"
  if command -v wget > /dev/null 2>&1; then
    wget --show-progress --progress=bar:force:noscroll -qO "$3" "$2"
  elif command -v curl > /dev/null 2>&1; then
    curl -Lf "$2" -o "$3"
  else
    printf "Unable to download required file: %s\n" "$2"
    exit 1
  fi
}

# Parameters:
# 1. Informative name
# 2. archive path
# 3. destination path
extract_tar_gz() {
  printf "Extracting %s\n" "$1"
  mkdir -p "$3"
  if command -v tar > /dev/null 2>&1; then
    tar -xf "$2" -C "$3" --strip-components=1
  elif command -v bsdtar > /dev/null 2>&1; then
    bsdtar -xf "$2" -C "$3" --strip-components=1
  else
    printf "Unable to extract required file: %s\n" "$2"
    exit 1
  fi
}

if [ "$#" -eq 1 ] && [ "$1" = "-f" ]; then
  rm -rf "$haxelib_dir" "$haxe_dir" "$neko_dir" out
fi

if [ ! -d "$haxe_dir" ]; then
  download_file Haxe https://github.com/HaxeFoundation/haxe/releases/download/$haxe_version/haxe-$haxe_version-linux64.tar.gz haxe.tar.gz
  extract_tar_gz Haxe haxe.tar.gz "$haxe_dir"
  rm haxe.tar.gz
  printf "\n"
  need_haxe=true
fi

if [ ! -d "$neko_dir" ]; then
  download_file Neko https://github.com/HaxeFoundation/neko/releases/download/v$neko_version_2/neko-$neko_version-linux64.tar.gz neko.tar.gz
  extract_tar_gz Neko neko.tar.gz "$neko_dir"
  rm neko.tar.gz
  printf "\n"
  need_neko=true
fi

if [ ! -d "$haxelib_dir" ]; then
  printf "Creating haxelib repo\n"
  LD_LIBRARY_PATH="$neko_dir:$LD_LIBRARY_PATH" "$haxe_dir"/haxelib setup "$haxelib_dir"
  printf "\n"
fi

if [ ! -d "$haxelib_dir"/hxcpp/$hxlib_version_2 ]; then
  printf "Downloading hxcpp\n"
  LD_LIBRARY_PATH="$neko_dir:$LD_LIBRARY_PATH" "$haxe_dir"/haxelib install hxcpp $hxlib_version
  printf "\n"
fi

if [ $need_haxe = true ] || ! command -v haxe > /dev/null 2>&1; then
  PATH="$haxe_dir:$PATH"
  need_subshell=true
fi

if [ $need_neko = true ] || ! command -v neko > /dev/null 2>&1; then
  PATH="$neko_dir:$PATH"
  LD_LIBRARY_PATH="$neko_dir:$LD_LIBRARY_PATH"
  need_subshell=true
fi

if [ $need_subshell = true ]; then
  printf "Launching subshell\n"
  LD_LIBRARY_PATH="$LD_LIBRARY_PATH" $SHELL
fi
