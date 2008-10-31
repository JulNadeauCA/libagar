#!/bin/sh
# Public domain

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
	echo "-D_GNU_SOURCE=1 -D_THREAD_SAFE -I${PREFIX}/include/agar -I${PREFIX}/include/SDL -I${PREFIX}/include/freetype2 -I${PREFIX}/include -I/usr/local/include/SDL -I/usr/local/include/freetype2 -I/usr/local/include -I/opt/local/include/SDL -I/opt/local/include/freetype2 -I/opt/local/include -I/usr/X11R6/include/SDL -I/usr/X11R6/include/freetype2 -I/usr/X11R6/include"
fi
if [ "$1" = "--libs" ]; then
	echo "-L${PREFIX}/lib -L/opt/local/lib -L/usr/local/lib -L/usr/X11R6/lib -lag_gui -lag_core -lSDLmain -lSDL -Wl,-framework,Cocoa -lfreetype -lz -framework OpenGL -lm -lpthread"
fi
