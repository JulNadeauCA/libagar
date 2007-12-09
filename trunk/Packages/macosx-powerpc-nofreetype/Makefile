# Public domain

PLATFORM=	unix
OS=		macosx
ARCH=		powerpc
FLAVOR=		-nofreetype

include ../Makefile.inc

do-build:
	(cd ${BUILDDIR} && ${SRCDIR}/configure \
	    --srcdir=${SRCDIR} --prefix=${FAKEDIR}/${PKGNAME} \
	    --enable-{threads,warnings,network,server} \
	    --with-gl --without-freetype --with-manpages --without-jpeg && \
	    make depend all install)
