#!/bin/sh
#
# Static agar-config for Linux/amd64 binary package.
# Public domain
#

PREFIX="%PREFIX%"

if [ "$1" = "--version" ]; then
	echo "%INSTALLED_VERSION%"
fi
if [ "$1" = "--release" ]; then
	echo "%INSTALLED_RELEASE%"
fi
if [ "$1" = "--prefix" ]; then
	echo "${PREFIX}"
fi
if [ "$1" = "--cflags" ]; then
	echo "-I${PREFIX}/include/agar -I${PREFIX}/include/SDL -I/usr/include/SDL -I/usr/local/include/SDL -I/usr/X11R6/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/X11R6/include"
fi
if [ "$1" = "--libs" ]; then
	echo "-L${PREFIX}/lib -L${PREFIX}/lib64 -L/usr/local/lib -L/usr/X11R6/lib -lag_gui -lag_core -lag_net -lSDL -lGL -lm -lpthread"
fi
