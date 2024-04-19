# Untitled Text Adventure

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
