#!/bin/sh
#
# $Csoft: install-includes.sh,v 1.1 2004/04/25 02:11:03 vedge Exp $
# Public domain
#

cd $1
INCLS=`ls -1 *.h 2>/dev/null`

if [ "$INCLS" != "" ]; then
	if [ ! -d "$2" ]; then
		echo "$INSTALL_INCL_DIR $2"
		$INSTALL_INCL_DIR $2
	fi
	for INCL in $INCLS; do
		echo "$INSTALL_INCL $INCL $2"
		$INSTALL_INCL $INCL $2
	done
fi

