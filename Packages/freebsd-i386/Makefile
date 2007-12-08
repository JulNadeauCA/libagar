# Public domain

PLATFORM=	unix
OS=		freebsd
ARCH=		i386
FLAVOR=

include ../Makefile.inc

do-build:
	(cd ${BUILDDIR} && ${SRCDIR}/configure \
	    --srcdir=${SRCDIR} --prefix=${FAKEDIR}/${PKGNAME} \
	    --enable-threads --enable-warnings \
	    --enable-network --enable-server \
	    --with-gl --with-freetype --without-jpeg --with-manpages && \
	    make depend all install)
