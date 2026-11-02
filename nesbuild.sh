#!/usr/bin/env sh

if ! command -v vasm6502_oldstyle >/dev/null 2>&1
then
	echo "vasm6502_oldstyle is not installed!"
	echo "Download from http://sun.hasenbraten.de/vasm/index.php?view=relsrc and build"
	echo "with \`make CPU=6502 SYNTAX=oldstyle\`."
else
	mkdir -p bin
	vasm6502_oldstyle src/nes/uta.asm -chklabels -nocase -Fbin -o "bin/uta.nes"
fi
