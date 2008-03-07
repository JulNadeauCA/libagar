#!/bin/sh
#
#	$Csoft: dist.sh,v 1.17 2004/03/13 09:26:44 vedge Exp $

VER=`date +%m%d%Y`
DISTFILE=agar-${VER}

cd ..
echo "packaging source: agar-${VER}"
rm -fr agar-${VER}
cp -fRp agar agar-${VER}

rm -fR `find agar-${VER} \( -name .svn \
    -or -name \*~ \
    -or -name \*.o \
    -or -name \*.a \
    -or -name \*.core \
    -or -name .\*.swp \
    -or -name .depend \
    -or -name .xvpics \)`

tar -f ${DISTFILE}.tar -c agar-${VER}
gzip -f ${DISTFILE}.tar
md5 ${DISTFILE}.tar.gz > ${DISTFILE}.tar.gz.md5
rmd160 ${DISTFILE}.tar.gz >> ${DISTFILE}.tar.gz.md5
sha1 ${DISTFILE}.tar.gz >> ${DISTFILE}.tar.gz.md5
gpg -ab ${DISTFILE}.tar.gz

