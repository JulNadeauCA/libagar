#!/bin/sh
#
#	$Csoft: dist.sh,v 1.10 2003/05/19 02:14:07 vedge Exp $

DATE=`date +%m%d%Y`
DISTFILE=agar-${DATE}

cd ..
echo "snapshot: agar-${DATE}"
rm -fr agar-${DATE}
cp -fRp agar agar-${DATE}
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
scp -C ${DISTFILE}.tar.{gz,gz.md5,gz.asc} vedge@resin:www/snap
ssh vedge@resin "cp -f www/snap/${DISTFILE}.tar.{gz,gz.md5,gz.asc} www/beta.csoft.org/agar && ls -l www/beta.csoft.org/agar/${DISTFILE}.tar.{gz,gz.md5,gz.asc}"

echo "notifying agar-announce@"
sh agar/mk/announce.sh ${DATE}

