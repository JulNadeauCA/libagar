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
	echo "-I${PREFIX}/include/agar -I${PREFIX}/include/SDL -I/usr/local/include/SDL -I${PREFIX}/include/freetype2 -I/usr/local/include/freetype2 -I${PREFIX}/include -I/usr/local/include -I/usr/include/mingw -D_GNU_SOURCE=1 -Dmain=SDL_main"
fi
if [ "$1" = "--libs" ]; then
	echo "-L${PREFIX}/lib -lag_gui -lag_core -L/usr/local/lib -lmingw32 -lSDLmain -lSDL -mwindows -lfreetype -lopengl32 -lm -lpthreadGCE2"
fi
