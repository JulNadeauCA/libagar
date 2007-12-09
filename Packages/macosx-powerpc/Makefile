# Public domain

PLATFORM=	unix
OS=		macosx
ARCH=		powerpc
FLAVOR=

include ../Makefile.inc

do-build:
	(cd ${BUILDDIR} && ${SRCDIR}/configure \
	    --srcdir=${SRCDIR} --prefix=${FAKEDIR}/${PKGNAME} \
	    --enable-{threads,warnings,network,server} \
	    --with-gl --with-freetype --with-manpages --without-jpeg && \
	    make depend all install)
