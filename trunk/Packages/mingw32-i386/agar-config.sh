#!/bin/sh
#
# Static agar-config for MinGW32/i386 binary package.
# Public domain
#

PREFIX="%PREFIX%"

if [ "$1" = "--version" ]; then echo "%INSTALLED_VERSION%"; fi
if [ "$1" = "--release" ]; then echo "%INSTALLED_RELEASE%"; fi
if [ "$1" = "--prefix" ]; then echo "${PREFIX}"; fi

if [ "$1" = "--threads" ]; then echo "yes"; fi
if [ "$1" = "--network" ]; then echo "no"; fi
if [ "$1" = "--sdl" ]; then echo "yes"; fi
if [ "$1" = "--opengl" ]; then echo "yes"; fi
if [ "$1" = "--freetype" ]; then echo "yes"; fi

if [ "$1" = "--cflags" ]; then
	echo "-I${PREFIX}/include/agar -I${PREFIX}/include/SDL -I${PREFIX}/include -I/usr/local/include/SDL -I/usr/local/include -I/usr/include/mingw -mno-cygwin -Dmain=SDL_main"
fi
if [ "$1" = "--libs" ]; then
	echo "-L${PREFIX}/lib -lag_gui -lag_core -L/usr/local/lib -lmingw32 -lSDLmain -lSDL -mno-cygwin -mwindows -lm"
fi
