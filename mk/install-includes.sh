#!/bin/sh
#
# $Csoft$
# Public domain
#

cd $1
INCLS=`ls -1 *.h 2>/dev/null`

if [ "$INCLS" != "" ]; then
	echo "$INSTALL_INCL_DIR $2"
	$INSTALL_INCL_DIR $2
	for INCL in $INCLS; do
		echo "$INSTALL_INCL $INCL $2"
		$INSTALL_INCL $INCL $2
	done
fi

