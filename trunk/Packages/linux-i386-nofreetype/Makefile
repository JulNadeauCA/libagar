# Public domain

PLATFORM=	unix
OS=		linux
ARCH=		i386
FLAVOR=		-nofreetype

include ../Makefile.inc

do-build:
	(cd ${BUILDDIR} && ${SRCDIR}/configure \
	    --srcdir=${SRCDIR} --prefix=${FAKEDIR}/${PKGNAME} \
	    --enable-{threads,network,server} \
	    --with-gl --without-freetype --with-manpages --without-jpeg && \
	    make depend all install)
