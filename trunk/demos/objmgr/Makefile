#	$Csoft: Makefile,v 1.1 2005/09/28 15:46:13 vedge Exp $

TOP=	../..
include Makefile.config

PROG=	objmgr
SRCS=	objmgr.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | manuconf > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/csoft.prog.mk
