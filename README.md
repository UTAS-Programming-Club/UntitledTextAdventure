# Untitled Text Adventure

## Targets
The plan is to implement the game in c with three frontends:
1. Console program  
   This will likely need full unicode and ansi sequences as we do not wanted to be limited in usable characters and want to have some support for simple images displayed via a image to text library which requires colour text printing.
   If we have a release build for this it will probably be built with [Cosmopolitan](https://github.com/jart/cosmopolitan) for compatibility with:
   * FreeBSD(aarch64, x86_64)
   * Linux(aarch64, x86_64)
   * MacOS(aarch64, x86_64)
   * NetBSD(x86_64)
   * OpenBSD(x86_64)
   * Windows 10/11(x86_64) with Windows Terminal, we are looking into using [mintty](https://github.com/mintty/mintty) to support older versions of Windows but have had some issues

   However we may not be able to test all of these platforms ourselves.

2. Windows GDI program  
   NOTE: This frontend is proving to be an issue to get working and may be replaced with mintty in combination with the command line one.  
   This is mostly to work around issues with displaying Unicode characters outside of the BMP on Windows.
   We plan to use [libschrift](https://github.com/tomolt/libschrift) for Unicode support on < Windows 2000. Will also be useful for Windows 2000 and Windows XP where non BMP character printing is supported but disabled on some releases without registry editing.
   A prototype of text printing has been tested on both Windows NT 4.0 and Windows XP so provided we can made everything else work most(if not all) versions of Windows can be supported with minimal extra work.
3. Discord bot  
   The club has been working on a [discord bot](https://github.com/UTAS-Programming-Club/DiscordBot), and we are interested in having a (potentially) cut-down version of the game available as a command.  
   An wrapper for an out of date version of the backend is currently available in that repo.

We have only just started so everything we have is a bit rough. So any contributions are welcome. There are a number of TODOs in various places for you don't know what to work on.

## General TODOs
* Embed GameData.json in unix like cmd build
* Switch gdi to libschrift for text rendering for < vista rendering(2000 and xp can work as is but not worth it)
* Add python/crescent frontend
* Add typedefs for state variable ids with defined invalid values
* Build instructions for release on windows with cosmo
* Fix pwsh debug build instructions to work on older versions, currently the &&s fails
* Support building tools and cmdgame with cosmo on OSes other than Windows

## Downloading repo
```sh
git clone --recurse-submodules https://github.com/UTAS-Programming-Club/UntitledTextAdventure.git
```
Optionally with -jSOME_NUMBER to download submodules in parallel.
Also need to make sure git keeps lf line endings but the entries in .gitattributes should deal with that.

## Basic build instructions
Notes:
* Debug game binaries for Windows and debug and release binaries for other OSes require GameData.json in the starting directory.
* Release game binaries for Windows with MinGW-w64 pack the json file into the binary.
* Only tested with GCC but should work with Clang. Unlikely to work with MSVC as is.
* Requires GNU Make which is provided for the cosmocc build. We accept contributions to support other versions of make.
* If any changes do not appear in the game after a rebuild, try cleaning first with `make clean`/`.\make.bat clean`.
* The Discord and GDI builds are often broken and may not even build, fixes are welcome.
* Add -jSOME_NUMBER to build in parallel. This is currently ignored when building zstd with cosmocc on Windows.
* The below example commands are for x86_64 but other arches should be fine as well, open an issue if something does not work.
* All builds require a C compiler, release builds with MinGW-w64 need windres and tool builds needs a C++ compiler.

<details>
<summary>Building for Unix-likes</summary>

Tested on Linux, hopefully works on other Unix-likes and possibly even Cygwin and MSYS2.
<br><br>

Debug
```sh
make CC=gcc debug               # Produces ./out/x86_64-pc-linux-gnu/debug/bin/cmdgame
make CC=gcc CXX=g++ debug-tools # Produces ./out/x86_64-pc-linux-gnu/debug/bin/{jsonvalidator, mapwatch, printgamedata}
```
Release:
```sh
make CC=gcc release       # Produces ./out/x86_64-pc-linux-gnu/release/bin/cmdgame
make CC=gcc discord       # Produces ./out/x86_64-pc-linux-gnu/release/bin/game.so
make CC=gcc CXX=g++ tools # Produces ./out/x86_64-pc-linux-gnu/release/bin/{jsonvalidator, mapwatch, printgamedata}
```

</details>

<details>
<summary>Building for Windows with MinGW-w64</summary>

Tested via cross compilation on WSL, hopefully also works on Windows directly including via MSYS2's version of MinGW-w64.  
If building on WSL, copying and running from Windows is recommended to avoid a startup speed penalty.
<br><br>

Debug:
```sh
make CC=x86_64-w64-mingw32-gcc debug                                  # Produces out/x86_64-w64-mingw32/debug/bin/{cmdgame.exe, gdigame.exe}
make CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ debug-tools # Produces out/x86_64-w64-mingw32/debug/bin/{jsonvalidator.exe, mapwatch.exe, printgamedata.exe}
```
Release:
```sh
make CC=x86_64-w64-mingw32-gcc WINDRES=x86_64-w64-mingw32-windres release # Produces out/x86_64-w64-mingw32/release/bin/{cmdgame.exe, gdigame.exe}
make CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ tools           # Produces out/x86_64-w64-mingw32/release/bin/{jsonvalidator.exe, mapwatch.exe, printgamedata.exe}
```

</details>

<details>
<summary>Building for Windows with cosmocc</summary>

Tested on Windows with the newest tools as of 20240711.  
For older versions of powershell either run the commands seperately or switch to cmd or powershell core.
<br><br>

Debug:
```pwsh
.\getwindeps.bat && .\make.bat debug       # Produces out/x86_64-pc-linux-cosmo/debug/bin/{cmdgame.com, gdigame.com}
.\getwindeps.bat && .\make.bat debug-tools # Produces out/x86_64-pc-linux-cosmo/debug/bin/{jsonvalidator.com, mapwatch.com, printgamedata.com}
```
Release:
```pwsh
.\getwindeps.bat && .\make.bat release # Produces out/x86_64-pc-linux-cosmo/release/bin/{cmdgame.com, gdigame.com}
.\getwindeps.bat && .\make.bat tools   # Produces out/x86_64-pc-linux-cosmo/release/bin/{jsonvalidator.com, mapwatch.com, printgamedata.com}
```

</details>
