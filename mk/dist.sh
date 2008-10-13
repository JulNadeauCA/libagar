#!/bin/sh
# Public domain

PROJ=agar
VER=`perl mk/get-version.pl`
REL=`perl mk/get-release.pl`
DISTNAME=${PROJ}-${VER}
RHOST=resin.csoft.net
RUSER=vedge
MAKE=make

if [ "$1" != "" ]; then
	PHASE="$1"
else
	PHASE=stable
fi
REMOTEDIR=www/${PHASE}.hypertriton.com/${PROJ}

cd ..

echo "*"
echo "* Project: ${PROJ}"
echo "* State: ${PHASE}"
echo "* Version: ${VER}"
echo "* Release: ${REL}"
echo "*"

# TAR: Prepare archive
echo "Building tar.gz"
rm -fr ${DISTNAME}
cp -fRp ${PROJ} ${DISTNAME}
rm -fR `find ${DISTNAME} \( -name .svn \
    -or -name \*~ \
    -or -name \*.o \
    -or -name \*.a \
    -or -name \*.core \
    -or -name .\*.swp \
    -or -name .xvpics \)`

# TAR: Compress archive
tar -f ${DISTNAME}.tar -c ${DISTNAME}
gzip -f ${DISTNAME}.tar

# ZIP: Prepare archive
echo "Building zip"
rm -fr ${DISTNAME}
cp -fRp ${PROJ} ${DISTNAME}
rm -fR `find ${DISTNAME} \( -name .svn \
    -or -name \*~ \
    -or -name \*.o \
    -or -name \*.a \
    -or -name \*.core \
    -or -name .\*.swp \
    -or -name .xvpics \)`
rm -f ${DISTNAME}/agar

# ZIP: Generate project files for various IDEs.
(cd ${DISTNAME} && ${MAKE} proj)
(cd ${DISTNAME}/demos && ${MAKE} proj)

# ZIP: Litter header files with "DECLSPEC" as required by some compilers.
(cd ${DISTNAME} && find . -name \*.h -exec perl mk/gen-declspecs.pl {} \;)

# ZIP: Compress archive
zip -q -r ${DISTNAME}.zip ${DISTNAME}

echo "Updating checksums"
openssl md5 ${DISTNAME}.tar.gz > ${DISTNAME}.tar.gz.md5
openssl rmd160 ${DISTNAME}.tar.gz >> ${DISTNAME}.tar.gz.md5
openssl sha1 ${DISTNAME}.tar.gz >> ${DISTNAME}.tar.gz.md5
openssl md5 ${DISTNAME}.zip > ${DISTNAME}.zip.md5
openssl rmd160 ${DISTNAME}.zip >> ${DISTNAME}.zip.md5
openssl sha1 ${DISTNAME}.zip >> ${DISTNAME}.zip.md5

echo "Updating signatures"
gpg -ab ${DISTNAME}.tar.gz
gpg -ab ${DISTNAME}.zip

echo "Uploading to ${RHOST}"
scp -C ${DISTNAME}.{tar.gz,tar.gz.md5,tar.gz.asc,zip,zip.md5,zip.asc} ${RUSER}@${RHOST}:${REMOTEDIR}

if [ "$PHASE" = "stable" ]; then
	echo "*********************************************************"
	echo "TODO:"
	echo "- Update http://en.wikipedia.org/wiki/Agar (software)"
	echo "- Update http://fr.wikipedia.org/wiki/Agar (moteur)"
	echo "- Update http://sourceforge.net/projects/agar/"
	echo "- Update http://freshmeat.net/projects/agar/"
	echo "- Announcements to agar@ and agar-fr@"
	echo "- Announcements to agar-announce@ and agar-announce-fr@"
	echo "*********************************************************"
fi
