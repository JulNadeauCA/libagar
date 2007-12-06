# Public domain

PLATFORM=	unix
OS=		cygwin
ARCH=		i386
FLAVOR=		-nofreetype_nogl_nothreads

include ../Makefile.inc

do-build:
	(cd ${BUILDDIR} && ${SRCDIR}/configure \
	    --srcdir=${SRCDIR} --prefix=${FAKEDIR}/${PKGNAME} \
	    --disable-threads --disable-network --disable-server \
	    --without-gl --without-freetype --without-jpeg \
	    --with-manpages && \
	    make depend all install)
