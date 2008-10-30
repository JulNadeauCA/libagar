#!/bin/sh
# Public domain

PREFIX="%PREFIX%"

if [ "$1" = "--version" ]; then echo "%INSTALLED_VERSION%"; fi
if [ "$1" = "--release" ]; then echo "%INSTALLED_RELEASE%"; fi
if [ "$1" = "--prefix" ]; then echo "${PREFIX}"; fi

if [ "$1" = "--threads" ]; then echo "yes"; fi
if [ "$1" = "--network" ]; then echo "yes"; fi

if [ "$1" = "--cflags" ]; then
	echo "-D_REENTRANT -D_GNU_SOURCE=1 -I${PREFIX}/include/agar -I${PREFIX}/include/SDL -I${PREFIX}/include -I/usr/local/include/SDL -I/usr/local/include"
fi
if [ "$1" = "--libs" ]; then
	echo "-L${PREFIX}/lib -L/usr/local/lib -lag_core -Wl,-rpath,/usr/local/lib -lSDL -pthread -Wl,--rpath -Wl,/usr/local/lib -lm -lpthread"
fi
