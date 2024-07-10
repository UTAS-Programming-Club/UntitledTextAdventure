# Untitled Text Adventure

## Targets:
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

## General TODOs:
* Embed GameData.json in unix like cmd build
* Switch gdi to libschrift for text rendering for < vista rendering(2000 and xp can work as is but not worth it)
* Add python/crescent frontend
* Add typedefs for screen, button and state variable ids with defined invalid values
* Build instructions for tools and release on windows for both cmd and pwsh, and discord bot
* Fix pwsh debug build instructions to work on older versions, currently the &&s fails
* Figure out why newer cosmo build tools fail hang on windows. It might just be make as it has had issues in the past
* Support building tools and cmdgame with cosmo on OSes other than Windows
* Hide "fatal error: no input files" errors at the top of all make calls

## Basic build instructions
All of these are build from wsl with windows builds copied to windows first to avoid very slow startup. Additionally they clean the project and then clear the screen which I find makes reading the build output easier for debugging but is probably not desired by others. They will need to be modified for other systems.
Debug game binaries for windows and release binaries for other oses require GameData.json in the starting directory.
Release game binaries for windows pack the json file into the binary.
Only tested with gcc but should work with clang. Unlikely to work with msvc as is.
Requires gnu make, will accept contributions to support other versions of make.

### Downloading repo
```sh
git clone --recurse-submodules https://github.com/UTAS-Programming-Club/UntitledTextAdventure.git
```
Optionally with -jSOME_NUMBER to download submodules in parallel.
Also need to make sure git keeps lf line endings but the entries in .gitattributes should deal with that.

### Building
Notes:
* If switching from a debug build to a release build, run `make clean`/`./make.bat clean` first.
* If any changes do not make it to the game, also try cleaning first.
* The discord and gdi builds are often broken and may not even build, fixes are welcome.

Copying wsl mingw output to windows is to get around wsl being slow at starting exes on it's own fs.
Debug on wsl(making windows binaries):
```sh
clear && make CC=x86_64-w64-mingw32-gcc debug && cp out/x86_64-w64-mingw32/bin/cmdgame.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/cmdgame.exe
clear && make CC=x86_64-w64-mingw32-gcc debug && cp out/x86_64-w64-mingw32/bin/gdigame.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/gdigame.exe
```

For cmd replace `clear` with `cls`.
For older versions of powershell either run the commands seperately or switch to cmd or powershell core.
Debug on windows 10/11 in powershell core:
```pwsh
.\getwindeps.bat && clear && .\make.bat debug && .\out\x86_64-pc-linux-cosmo\bin\cmdgame.com
.\getwindeps.bat && clear && .\make.bat debug && .\out\x86_64-pc-linux-cosmo\bin\gdigame.com
```

Release on wsl(making windows binaries):
```sh
clear && make CC=x86_64-w64-mingw32-gcc WINDRES=x86_64-w64-mingw32-windres release && cp out/x86_64-w64-mingw32/bin/cmdgame.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/cmdgame.exe
clear && make CC=x86_64-w64-mingw32-gcc WINDRES=x86_64-w64-mingw32-windres release && cp out/x86_64-w64-mingw32/bin/gdigame.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/gdigame.exe
```

Release on linux, hopefully other unix likes and possibly cygwin or msys2:
```sh
clear && make release && ./out/x86_64-pc-linux-gnu/bin/cmdgame
clear && make discord
```

Tools on wsl(making windows binaries):
```sh
clear && make CC=x86_64-w64-mingw32-gcc-10-win32 CXX=x86_64-w64-mingw32-g++-win32 tools && cp out/x86_64-w64-mingw32/bin/jsonvalidator.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/jsonvalidator.exe
clear && make CC=x86_64-w64-mingw32-gcc-10-win32 CXX=x86_64-w64-mingw32-g++-win32 tools && cp out/x86_64-w64-mingw32/bin/mapwatch.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/mapwatch.exe
clear && make CC=x86_64-w64-mingw32-gcc-10-win32 CXX=x86_64-w64-mingw32-g++-win32 tools && cp out/x86_64-w64-mingw32/bin/printgamedata.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/printgamedata.exe GameData.json
```

Tools on linux, hopefully other unix likes and possibly cygwin or msys2:
```sh
clear && make tools && ./out/x86_64-pc-linux-gnu/bin/jsonvalidator
clear && make tools && ./out/x86_64-pc-linux-gnu/bin/mapwatch
clear && make tools && ./out/x86_64-pc-linux-gnu/bin/printgamedata GameData.json
```

For cmd replace `clear` with `cls`.
For older versions of powershell either run the commands seperately or switch to cmd or powershell core.
Tools on windows 10/11 in powershell core:
```pwsh
.\getwindeps.bat && clear && .\make.bat tools && .\out\x86_64-pc-linux-cosmo\bin\jsonvalidator.com
.\getwindeps.bat && clear && .\make.bat tools && .\out\x86_64-pc-linux-cosmo\bin\mapwatch.com
.\getwindeps.bat && clear && .\make.bat tools && .\out\x86_64-pc-linux-cosmo\bin\printgamedata.com GameData.json
```
