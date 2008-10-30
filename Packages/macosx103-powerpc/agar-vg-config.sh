#!/bin/sh
# Public domain

PREFIX="%PREFIX%"

if [ "$1" = "--version" ]; then echo "%INSTALLED_VERSION%"; fi
if [ "$1" = "--release" ]; then echo "%INSTALLED_RELEASE%"; fi
if [ "$1" = "--prefix" ]; then echo "${PREFIX}"; fi

if [ "$1" = "--threads" ]; then echo "yes"; fi

if [ "$1" = "--cflags" ]; then
	echo "-I${PREFIX}/include/agar"
fi
if [ "$1" = "--libs" ]; then
	echo "-L${PREFIX}/lib -lag_vg"
fi
