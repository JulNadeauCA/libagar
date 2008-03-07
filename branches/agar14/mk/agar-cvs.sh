#!/bin/sh
#
#	$Csoft: agar-cvs.sh,v 1.1 2003/03/14 07:54:30 vedge Exp $

TMPDIR=/hack/vedge/.tmp
CVSREPO=/home/cvs/agar

(cd $CVSREPO &&
 tar -f- -c . | gzip -9 - > $TMPDIR/agar-cvs.tar.gz &&
 scp -qC $TMPDIR/agar-cvs.tar.gz vedge@resin:www/agar/cvs &&
 rm -f $TMPDIR/agar-cvs.tar.gz &&
 ssh vedge@resin 'cd ~/www/agar/cvs && tar -xzf agar-cvs.tar.gz && rm -f agar-cvs.tar.gz; /bin/sh ~vedge/bin/agar.sh')

