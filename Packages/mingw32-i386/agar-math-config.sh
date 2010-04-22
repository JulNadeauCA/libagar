#!/bin/sh
# Public domain

PREFIX="%PREFIX%"

if [ "$1" = "--version" ]; then echo "%INSTALLED_VERSION%"; fi
if [ "$1" = "--prefix" ]; then echo "${PREFIX}"; fi
if [ "$1" = "--exec-prefix" ]; then echo "${PREFIX}"; fi

if [ "$1" = "--cflags" ]; then
	echo "-I${PREFIX}/include/agar"
fi
if [ "$1" = "--libs" ]; then
	echo "-L${PREFIX}/lib -lag_math -lm"
fi
