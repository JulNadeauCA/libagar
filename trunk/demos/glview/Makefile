#	$Csoft: Makefile,v 1.1 2005/10/04 18:02:17 vedge Exp $

TOP=	../..
include Makefile.config

PROG=	glview
SRCS=	glview.c

CFLAGS+=${AGAR_CFLAGS} ${OPENGL_CFLAGS} ${GLU_CFLAGS} -I.
LIBS+=	${AGAR_LIBS} ${OPENGL_LIBS} ${GLU_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | manuconf > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/csoft.prog.mk
