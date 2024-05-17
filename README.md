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
   * Windows 10/11(x86_64) with Windows Terminal
   However we may not be able to test all of these platforms ourselves.
2. Windows GDI program  
   This is mostly to work around issues with displaying Unicode characters outside of the BMP on Windows.
   We plan to use [libschrift](https://github.com/tomolt/libschrift) for Unicode support on < Windows 2000. Will also be useful for Windows 2000 and Windows XP where non BMP character printing is supported but disabled on some releases without registry editing.
   A prototype of text printing has been tested on both Windows NT 4.0 and Windows XP so provided we can made everything else work most(if not all) versions of windows can be supported with minimal extra work.
3. Discord bot  
   The club has been working on a [discord bot](https://github.com/UTAS-Programming-Club/DiscordBot) and we are interested in having a (potentially) cut down version of the game available as a commmand.

We have only just started so everything we have is a bit rough. So any contributions are welcome. There are a number of TODOs in various places for you don't know what to work on.

## General TODOs:
* Trim comments from release GameData.json
* Embed GameData.json in unix like cmd build
* Switch gdi to libschrift for text rendering for < vista rendering(2000 and xp can work as is but not worth it)
* Add python/crescent frontend
* After adding libschrift switch to utf-8
* Add typedefs for screen, button and state variable ids with defined invalid values
* Build instructions for tools and release on windows for both cmd and pwsh, and discord bot
* Fix pwsh debug build instructions to work on older versions, currently the &&s fails
* Figure out why newer cosmo build tools fail hang on windows. It might just be make as it has had issues in the past

## Basic build instructions
All of these are build from wsl with windows builds copied to windows first to avoid very slow startup. Additionally they clean the project and then clear the screen which I find makes reading the build output easier for debugging but is probably not desired by others. They will need to be modified for other systems.
Debug game binaries for windows and release binaries for other oses require GameData.json in the starting directory.
Release game binaries for windows pack the json file into the binary.

Downloading repo:
```sh
git clone --recurse-submodules https://github.com/UTAS-Programming-Club/UntitledTextAdventure.git
```
Optionally with -jSOME_NUMBER to download submodules in parallel.

Copying wsl mingw output to windows is to get around wsl being slow at starting exes on it's own fs.
Debug on wsl(making windows binaries):
```sh
make clean && clear && make CC=x86_64-w64-mingw32-gcc-10-win32 debug && cp out/x86_64-w64-mingw32/bin/cmdgame.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/cmdgame.exe
make clean && clear && make CC=x86_64-w64-mingw32-gcc-10-win32 debug && cp out/x86_64-w64-mingw32/bin/gdigame.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/gdigame.exe
```

Debug on windows 10 in powershell core:
```pwsh
.\getwindeps.bat && .\make.bat clean && clear && .\make.bat debug && .\third_party\cosmos\bin\apelink.exe -l .\third_party\cosmos\bin\ape-x86_64.elf -o .\out\x86_64-pc-linux-cosmo\bin\cmdgame.com .\out\x86_64-pc-linux-cosmo\bin\cmdgame
.\getwindeps.bat && .\make.bat clean && clear && .\make.bat debug && .\third_party\cosmos\bin\apelink.exe -l .\third_party\cosmos\bin\ape-x86_64.elf -o .\out\x86_64-pc-linux-cosmo\bin\gdigame.com .\out\x86_64-pc-linux-cosmo\bin\gdigame
```

Release on wsl(making windows binaries):
```sh
make clean && clear && make CC=x86_64-w64-mingw32-gcc-10-win32 WINDRES=x86_64-w64-mingw32-windres release && cp out/x86_64-w64-mingw32/bin/cmdgame.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/cmdgame.exe
make clean && clear && make CC=x86_64-w64-mingw32-gcc-10-win32 WINDRES=x86_64-w64-mingw32-windres release && cp out/x86_64-w64-mingw32/bin/gdigame.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/gdigame.exe
```

Release on linux and hopefully other unix likes:
```sh
make clean && clear && make CC=gcc release && ./out/x86_64-pc-linux-gnu/bin/cmdgame
```

Tools on wsl(making windows binaries):
```sh
make clean && clear && make CC=x86_64-w64-mingw32-gcc-10-win32 CXX=x86_64-w64-mingw32-g++-win32 tools && cp out/x86_64-w64-mingw32/bin/preptext.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/preptext.exe "Some text"
make clean && clear && make CC=x86_64-w64-mingw32-gcc-10-win32 CXX=x86_64-w64-mingw32-g++-win32 tools && cp out/x86_64-w64-mingw32/bin/printgamedata.exe /mnt/c/Projects/PCGame/Windows/ && /mnt/c/Projects/PCGame/Windows/printgamedata.exe GameData.json
```

Tools on linux and hopefully other unix likes:
```sh
make clean && clear && make CC=gcc CXX=g++ tools && ./out/x86_64-pc-linux-gnu/bin/preptext "Some text"
make clean && clear && make CC=gcc CXX=g++ tools && ./out/x86_64-pc-linux-gnu/bin/printgamedata GameData.json
```
