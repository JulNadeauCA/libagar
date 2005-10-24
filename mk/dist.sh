#!/bin/sh
#
#	$Csoft: dist.sh,v 1.17 2004/03/13 09:26:44 vedge Exp $

VER=`grep "HDEFINE(VERSION" configure.in |awk -F\\" '{print $3}' |awk -F\\\ '{print $1}'`
PHASE=stable
DISTFILE=agar-${VER}
HOST=resin.csoft.net
RUSER=vedge
MAILER="sendmail -t"

echo "Packaging agar-${VER}"

cd ..
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
gzip -f9 ${DISTFILE}.tar
openssl md5 ${DISTFILE}.tar.gz > ${DISTFILE}.tar.gz.md5
openssl rmd160 ${DISTFILE}.tar.gz >> ${DISTFILE}.tar.gz.md5
openssl sha1 ${DISTFILE}.tar.gz >> ${DISTFILE}.tar.gz.md5
gpg -ab ${DISTFILE}.tar.gz

if [ "$1" = "commit" ]; then
	echo "uploading"
	scp -C ${DISTFILE}.{tar.gz,tar.gz.md5,tar.gz.asc} ${RUSER}@${HOST}:www/$PHASE.csoft.org/agar

	echo "notifying agar-announce@"
	TMP=`mktemp /tmp/agarannounceXXXXXXXX`
	cat > $TMP << EOF
From: Wilbern Cobb <vedge@csoft.org>
To: agar-announce@lists.csoft.net
Subject: New Agar release: $VER
X-Mailer: announce.sh
X-PGP-Key: 206C63E6

A new Agar release has been uploaded to $PHASE.csoft.org.

	http://$PHASE.csoft.org/agar/agar-$VER.tar.gz
	http://$PHASE.csoft.org/agar/agar-$VER.tar.gz.asc
	http://$PHASE.csoft.org/agar/agar-$VER.tar.gz.md5

EOF
	cat $TMP | ${MAILER}
	rm -f $TMP
fi
