# Public domain

PLATFORM=	unix
OS=		linux
ARCH=		amd64
FLAVOR=

include ../Makefile.inc

do-build:
	(cd ${BUILDDIR} && ${SRCDIR}/configure \
	    --srcdir=${SRCDIR} --prefix=${FAKEDIR}/${PKGNAME} \
	    --enable-{threads,warnings,network,server} \
	    --with-gl --with-freetype --with-manpages --without-jpeg && \
	    make depend all install)
