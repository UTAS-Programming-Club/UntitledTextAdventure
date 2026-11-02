#!/usr/bin/env sh

# every other method breaks
cp src/wii/Makefile .
make
rm Makefile
