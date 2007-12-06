# Public domain

PLATFORM=	unix
OS=		linux
ARCH=		mipsel
FLAVOR=		-nofreetype_nogl_nothreads

include ../Makefile.inc

do-build:
	(cd ${BUILDDIR} && ${SRCDIR}/configure \
	    --srcdir=${SRCDIR} --prefix=${FAKEDIR}/${PKGNAME} \
	    --enable-{network,server} \
	    --disable-threads \
	    --disable-warnings \
	    --without-gl --without-freetype --without-jpeg --with-manpages && \
	    make depend all install)
