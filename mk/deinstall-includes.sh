#!/bin/sh
# Public domain

DIR=$1
DEST=$2
cd $DIR
INCLS=`ls -1 *.h 2>/dev/null`
if [ "$INCLS" != "" ]; then
	for INCL in $INCLS; do
		echo "$DEINSTALL_INCL $DEST/$DIR/$INCL"
		$DEINSTALL_INCL $DESTDIR$DEST/$DIR/$INCL
	done
	if [ -e "$DESTDIR$DEST/$DIR" ]; then
		echo "$DEINSTALL_INCL_DIR $DEST/$DIR"
		$DEINSTALL_INCL_DIR $DESTDIR$DEST/$DIR
	fi
fi

