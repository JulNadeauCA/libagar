#!/bin/sh
#
#	$Csoft$

DATE=`date +%m%d%Y`
DISTFILE=agar-${DATE}

cd ..
echo "copying agar onto agar-${DATE}"
rm -fr agar-${DATE}
cp -fRp agar agar-${DATE}
rm -fR `find agar-${DATE} \( -name CVS \
    -or -name \*~ -or -name \*.o -or -name \*.core \
    -or -name ttf.c -or -name ttf.h -or -name oldxcf.c -or -name oldxcf.h \)`

echo "packaging"
tar -f ${DISTFILE}.tar -c agar-${DATE}
gzip -f9 ${DISTFILE}.tar
md5 ${DISTFILE}.tar.gz > ${DISTFILE}.tar.gz.md5
rmd160 ${DISTFILE}.tar.gz >> ${DISTFILE}.tar.gz.md5
sha1 ${DISTFILE}.tar.gz >> ${DISTFILE}.tar.gz.md5

scp ${DISTFILE}.tar.{gz,gz.md5}		vedge@resin:www/snap
scp ${DISTFILE}.tar.{gz,gz.md5}		vedge@resin:www/beta.csoft.org/agar

