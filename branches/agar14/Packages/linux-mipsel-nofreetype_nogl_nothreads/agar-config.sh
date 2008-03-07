#!/bin/sh
#
# Static agar-config for Linux/mipsel binary package.
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
	echo "-I${PREFIX}/include/agar -I${PREFIX}/include/SDL -I${PREFIX}/include -I/usr/local/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT"
fi
if [ "$1" = "--libs" ]; then
	echo "-L${PREFIX}/lib -lag_gui -lag_core -lag_net -L/usr/local/lib -Wl,-rpath,/usr/local/lib -Wl,-rpath,${PREFIX}/lib -lSDL -lpthread -lm"
fi
