#!/bin/sh
#
#	$Csoft: dist.sh,v 1.17 2004/03/13 09:26:44 vedge Exp $
#	Public domain

VER=`perl mk/get-version.pl`
REL=`perl mk/get-release.pl`
PHASE=stable
DISTFILE=agar-${VER}
HOST=resin.csoft.net
RUSER=vedge
MAILER="sendmail -t"

echo "Packaging agar-${VER} (${REL})"

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
gzip -f ${DISTFILE}.tar
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
From: Julien Nadeau <vedge@csoft.org>
To: agar-announce@lists.csoft.net
Subject: Agar-${VER} (${REL}) released
X-Mailer: dist.sh
X-PGP-Key: 206C63E6

We are pleased to announce the official release of agar ${VER}
(${REL}).

It is now available for download from ${PHASE}.csoft.org.

	http://$PHASE.csoft.org/agar/agar-$VER.tar.gz
	http://$PHASE.csoft.org/agar/agar-$VER.tar.gz.asc
	http://$PHASE.csoft.org/agar/agar-$VER.tar.gz.md5

Binary packages are also available from the agar website:

	http://agar.csoft.org/download.html.

Your comments, suggestions and bug reports are most welcome.
EOF
	cat $TMP | ${MAILER}

	echo "notifying agar-announce-fr@"
	TMP=`mktemp /tmp/agarannounceXXXXXXXX`
	cat > $TMP << EOF
From: Julien Nadeau <vedge@csoft.org>
To: agar-announce@lists.csoft.net
Subject: Sortie: Agar ${VER} (${REL})
X-Mailer: announce.sh
X-PGP-Key: 206C63E6

Il me fait plaisir d'annoncer la sortie officielle de agar ${VER}
(${REL}).

La distribution source est téléchargable à partir de ${PHASE}.csoft.org:

	http://$PHASE.csoft.org/agar/agar-$VER.tar.gz
	http://$PHASE.csoft.org/agar/agar-$VER.tar.gz.asc
	http://$PHASE.csoft.org/agar/agar-$VER.tar.gz.md5

Des paquets binaires sont également disponibles sur le site d'agar:

	http://agar.csoft.org/download.html.

Vos commentaires, suggestions et signalements de bogues sont, comme
toujours, fortement appréciés.
EOF
	cat $TMP | ${MAILER}
	rm -f $TMP
fi
