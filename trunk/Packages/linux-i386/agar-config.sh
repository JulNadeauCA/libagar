#!/bin/sh
#
# Static agar-config for Linux/i386 binary packages.
# Public domain
#

PREFIX="%PREFIX%"

if [ "$1" = "--version" ]; then echo "%INSTALLED_VERSION%"; fi
if [ "$1" = "--release" ]; then echo "%INSTALLED_RELEASE%"; fi
if [ "$1" = "--prefix" ]; then echo "${PREFIX}"; fi

if [ "$1" = "--threads" ]; then echo "yes"; fi
if [ "$1" = "--network" ]; then echo "yes"; fi
if [ "$1" = "--sdl" ]; then echo "yes"; fi
if [ "$1" = "--opengl" ]; then echo "yes"; fi
if [ "$1" = "--freetype" ]; then echo "yes"; fi

if [ "$1" = "--cflags" ]; then
	echo "-I${PREFIX}/include/agar -I${PREFIX}/include/SDL -I${PREFIX}/include/freetype2 -I/usr/include/SDL -I/usr/local/include/SDL -I/usr/X11R6/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/freetype2 -I/usr/local/include/freetype2 -I/usr/X11R6/include/freetype2 -I/usr/X11R6/include"
fi
if [ "$1" = "--libs" ]; then
	echo "-L${PREFIX}/lib -L/usr/local/lib -L/usr/X11R6/lib -L/usr/lib -lag_gui -lag_core -lag_net -lSDL -lfreetype -lz -lGL -lm -lpthread"
fi
