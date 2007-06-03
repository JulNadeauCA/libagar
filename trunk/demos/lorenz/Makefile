#	$Csoft: Makefile,v 1.1 2005/10/04 18:02:17 vedge Exp $

TOP=	../..
include Makefile.config

PROG=	lorenz
SRCS=	lorenz.c

CFLAGS+=${AGAR_CFLAGS} ${AGAR_SG_CFLAGS} -I.
LIBS+=	${AGAR_LIBS} ${AGAR_SG_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | manuconf > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/csoft.prog.mk
