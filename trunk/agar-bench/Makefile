#	$Csoft: Makefile,v 1.5 2005/10/03 17:38:40 vedge Exp $

TOP=	.
include ${TOP}/Makefile.config

PROG=	agar-bench
SRCS=	agar-bench.c generic.c pixelops.c primitives.c surfaceops.c memops.c
VERSION=1.0

CFLAGS+=${AGAR_CFLAGS} -I. -DHAVE_RDTSC -DVERSION=\"${VERSION}\"
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | manuconf > configure
	chmod 755 configure

release: cleandir
	sh mk/release.sh
	#sh mk/cvs.sh

.PHONY: configure release

include ${TOP}/mk/csoft.prog.mk
