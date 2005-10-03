#	$Csoft: Makefile,v 1.3 2005/10/03 06:15:45 vedge Exp $

TOP=	.
include ${TOP}/Makefile.config

PROG=	agar-bench
SRCS=	agar-bench.c generic.c pixelops.c primitives.c surfaceops.c

CFLAGS+=${AGAR_CFLAGS} -I. -DHAVE_RDTSC
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
