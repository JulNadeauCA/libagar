#!/bin/sh
#
#	$Csoft: dist.sh,v 1.11 2003/06/06 03:27:10 vedge Exp $

DATE=`date +%m%d%Y`
DISTFILE=agar-${DATE}
CVSROOT=/home/cvs/CVSROOT

cd ..
echo "snapshot: agar-${DATE}"
rm -fr agar-${DATE}
cp -fRp agar agar-${DATE}

(cd ${CVSROOT} &&
 mv -f Agar-ChangeLog Agar-ChangeLog-${DATE} &&
 touch Agar-ChangeLog &&
 chgrp csoft Agar-ChangeLog &&
 chmod 666 Agar-ChangeLog)

cp -f ${CVSROOT}/Agar-ChangeLog agar-${DATE}/ChangeLog-${DATE}
cp -f ${CVSROOT}/Agar-ChangeLog-${DATE} agar-${DATE}.ChangeLog

rm -fR `find agar-${DATE} \( -name CVS \
    -or -name \*~ \
    -or -name \*.o \
    -or -name \*.a \
    -or -name \*.core \
    -or -name .\*.swp \
    -or -name .depend \
    -or -name .xvpics \)`

echo "packaging"
tar -f ${DISTFILE}.tar -c agar-${DATE}
gzip -f9 ${DISTFILE}.tar
md5 ${DISTFILE}.tar.gz > ${DISTFILE}.tar.gz.md5
rmd160 ${DISTFILE}.tar.gz >> ${DISTFILE}.tar.gz.md5
sha1 ${DISTFILE}.tar.gz >> ${DISTFILE}.tar.gz.md5
gpg -ab ${DISTFILE}.tar.gz

echo "uploading"
scp -C ${DISTFILE}.{tar.gz,tar.gz.md5,tar.gz.asc,ChangeLog} vedge@resin:www/snap
ssh vedge@resin "cp -f www/snap/${DISTFILE}.{tar.gz,tar.gz.md5,tar.gz.asc,ChangeLog} www/beta.csoft.org/agar && ls -l www/beta.csoft.org/agar/${DISTFILE}.*"

echo "notifying agar-announce@"
sh agar/mk/announce.sh ${DATE}

