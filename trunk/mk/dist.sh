#!/bin/sh
# Public domain

PROJ=agar
VER=`perl mk/get-version.pl`
REL=`perl mk/get-release.pl`
DISTNAME=${PROJ}-${VER}
RHOST=resin
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

#
# Prepare Source TAR.GZ.
#
echo "Building tar.gz"
if [ -e "${DISTNAME}" ]; then
	echo "* Existing directory: ${DISTNAME}; remove first"
	exit 1
fi
cp -fRp ${PROJ} ${DISTNAME}
rm -fR `find ${DISTNAME} \( -name .svn -or -name \*~ -or -name .\*.swp \)`

# TAR: Prepare standard README.txt and friends.
(cd ${DISTNAME} && env PKG_OS="" ${MAKE} pre-package)

# TAR: Compress archive
tar -f ${DISTNAME}.tar -c ${DISTNAME}
gzip -f ${DISTNAME}.tar

#
# Prepare Source ZIP.
#
echo "Building zip"
rm -fr ${DISTNAME}
cp -fRp ${PROJ} ${DISTNAME}
rm -fR `find ${DISTNAME} \( -name .svn -or -name \*~ -or -name .\*.swp \)`

# ZIP: Preprocess header files.
(cd ${DISTNAME} && ${MAKE} includes)

# ZIP: Prepare IDE "project files", README.txt and friends.
(cd ${DISTNAME} && ${MAKE} proj)
(cd ${DISTNAME}/tests && ${MAKE} proj)
(cd ${DISTNAME} && env PKG_OS="windows" ${MAKE} pre-package)
(cd ${DISTNAME} && rm -f README RELEASE-${VER})

# ZIP: Compress archive
zip -8 -q -r ${DISTNAME}.zip ${DISTNAME}

echo "Updating checksums"
openssl md5 ${DISTNAME}.tar.gz > ${DISTNAME}.tar.gz.md5
openssl sha256 ${DISTNAME}.tar.gz >> ${DISTNAME}.tar.gz.md5
openssl md5 ${DISTNAME}.zip > ${DISTNAME}.zip.md5
openssl sha256 ${DISTNAME}.zip >> ${DISTNAME}.zip.md5

echo "Updating signatures"
gpg -ab ${DISTNAME}.tar.gz
gpg -ab ${DISTNAME}.zip

if [ "$NOUPLOAD" = "" ]; then
	echo "Upload to ${RHOST}?"
	read FOO
	if [ "$FOO" != "no" ]; then
		scp -C ${DISTNAME}.{tar.gz,tar.gz.md5,tar.gz.asc,zip,zip.md5,zip.asc} ${RHOST}:${REMOTEDIR}
	fi
fi

if [ "$PHASE" = "stable" ]; then
	echo "*********************************************************"
	echo "TODO:"
	echo "- Double check core/version.h and build.lib.mk versions"
	echo " "
	echo "- Create http://wiki.libagar.org/wiki/Agar-${VER}"
	echo "- Update http://wiki.libagar.org/wiki/Main_Page"
	echo "- Update http://sourceforge.net/projects/agar/"
	echo "- Notify agar@, agar-announce@, agar-announce-fr@"
	echo "- Notify @libAgar"
	echo "*********************************************************"
fi
