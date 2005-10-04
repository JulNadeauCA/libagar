#	$Csoft: Makefile,v 1.6 2005/10/04 05:52:22 vedge Exp $

TOP=	.
include ${TOP}/Makefile.config

PROG=	agar-bench
SRCS=	agar-bench.c generic.c pixelops.c primitives.c surfaceops.c memops.c \
	misc.c
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
