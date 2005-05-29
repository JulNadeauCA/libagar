#	$Csoft: Makefile,v 1.1 2005/05/10 12:22:07 vedge Exp $

TOP=	.
include ${TOP}/Makefile.config

PROG=	tests
SRCS=	tests.c pixelops.c primitives.c

CFLAGS+=${AGAR_CFLAGS} -I.
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
