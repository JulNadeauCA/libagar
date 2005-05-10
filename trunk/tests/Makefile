#	$Csoft: Makefile,v 1.5 2004/04/25 07:38:29 vedge Exp $

TOP=	.
include ${TOP}/Makefile.config

PROG=	tests
SRCS=	tests.c pixelops.c

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
