#!/bin/sh
# Public domain

cd $1
INCLS=`ls -1 *.h 2>/dev/null`
if [ "$INCLS" != "" ]; then
	if [ ! -d "$DESTDIR$2/$1" ]; then
		echo "$INSTALL_INCL_DIR $2/$1"
		$INSTALL_INCL_DIR $DESTDIR$2/$1
	fi
	for INCL in $INCLS; do
		echo "$INSTALL_INCL $INCL $2/$1"
		$INSTALL_INCL $INCL $DESTDIR$2/$1
	done
fi

