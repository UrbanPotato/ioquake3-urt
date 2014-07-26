#!/bin/sh

export CC=x86_64-w64-mingw32-gcc.exe
export WINDRES=x86_64-w64-mingw32-windres.exe
export PLATFORM=mingw32
export ARCH=x86_64
exec make $*
