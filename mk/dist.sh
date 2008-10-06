#!/bin/sh
# Public domain

PROJ=agar
VER=`perl mk/get-version.pl`
REL=`perl mk/get-release.pl`
DISTNAME=${PROJ}-${VER}
RHOST=resin.csoft.net
RUSER=vedge
MAKE=make
REMOTEDIR=www/${PHASE}.csoft.org/${PROJ}

if [ "$1" != "" ]; then
	PHASE="$1"
else
	PHASE=stable
fi

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

# ZIP: Generate project files for various IDEs.
(cd ${DISTNAME} && ${MAKE} proj)
if [ $? != 0 ]; then
	echo "${MAKE} proj failed"
	exit 1
fi
(cd ${DISTNAME}/demos && ${MAKE} proj)
if [ $? != 0 ]; then
	echo "${MAKE} proj (demos) failed"
	exit 1
fi

# ZIP: Remove include symlink.
rm -f ${DISTNAME}/agar

# ZIP: Litter header files with "DECLSPEC" as required by some compilers.
find ${DISTNAME} -name \*.h -exec perl mk/gen-declspecs.pl {} \;

# ZIP: Compress archive
zip -r ${DISTNAME}.zip ${DISTNAME}

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

if [ "$1" = "commit" -o "$1" = "snapshot" ]; then
	echo "Uploading to ${RHOST}"
	scp -C ${DISTNAME}.{tar.gz,tar.gz.md5,tar.gz.asc,zip,zip.md5,zip.asc} ${RUSER}@${RHOST}:${REMOTEDIR}
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
