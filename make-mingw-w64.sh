#!/bin/sh
[ ! -d "$MINGW" ] && export MINGW=$HOME/mingw
[ ! -d "$MINGW" ] && export MINGW=$HOME/src/mingw
[ ! -d "$MINGW" ] && echo Please set MINGW directory first && exit 1
export CXX=i686-w64-mingw32-g++
make $@
