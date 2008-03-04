#!/bin/sh
# Public domain

PROJ=agar
VER=`perl mk/get-version.pl`
REL=`perl mk/get-release.pl`
DISTFILE=${PROJ}-${VER}
RHOST=resin.csoft.net
RUSER=vedge

if [ "$1" = "snapshot" ]; then
	PHASE=beta
else
	PHASE=stable
fi

echo "Packaging ${PROJ}-${VER} (${REL})"

cd ..
rm -fr ${PROJ}-${VER}
cp -fRp ${PROJ} ${PROJ}-${VER}
rm -fR `find ${PROJ}-${VER} \( -name .svn \
    -or -name \*~ \
    -or -name \*.o \
    -or -name \*.a \
    -or -name \*.core \
    -or -name .\*.swp \
    -or -name .depend \
    -or -name .xvpics \)`

tar -f ${DISTFILE}.tar -c ${PROJ}-${VER}
gzip -f ${DISTFILE}.tar
zip -r ${DISTFILE}.zip ${PROJ}-${VER}

openssl md5 ${DISTFILE}.tar.gz > ${DISTFILE}.tar.gz.md5
openssl rmd160 ${DISTFILE}.tar.gz >> ${DISTFILE}.tar.gz.md5
openssl sha1 ${DISTFILE}.tar.gz >> ${DISTFILE}.tar.gz.md5
openssl md5 ${DISTFILE}.zip > ${DISTFILE}.zip.md5
openssl rmd160 ${DISTFILE}.zip >> ${DISTFILE}.zip.md5
openssl sha1 ${DISTFILE}.zip >> ${DISTFILE}.zip.md5
gpg -ab ${DISTFILE}.tar.gz
gpg -ab ${DISTFILE}.zip

if [ "$1" = "commit" -o "$1" = "snapshot" ]; then
	echo "uploading"
	scp -C ${DISTFILE}.{tar.gz,tar.gz.md5,tar.gz.asc,zip,zip.md5,zip.asc} ${RUSER}@${RHOST}:www/$PHASE.csoft.org/${PROJ}
fi

echo "*********************************************************"
echo "TODO: Update http://en.wikipedia.org/wiki/Agar (software)"
echo "TODO: Update http://fr.wikipedia.org/wiki/Agar (moteur)"
echo "TODO: Update http://sourceforge.net/projects/agar/"
echo "TODO: Update http://freshmeat.net/projects/agar/"
echo "TODO: Announcements to agar@ and agar-fr@"
echo "TODO: Announcements to agar-announce@ and agar-announce-fr@"
echo "*********************************************************"

