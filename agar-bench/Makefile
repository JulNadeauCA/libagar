#	$Csoft: Makefile,v 1.7 2005/10/04 16:12:31 vedge Exp $

TOP=	..
include ./Makefile.config

PROG=	agar-bench
SRCS=	agar-bench.c generic.c pixelops.c primitives.c surfaceops.c memops.c \
	misc.c events.c
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
