TOP=	../..
include Makefile.config

PROG=	catastrophe
SRCS=	catastrophe.c

CFLAGS+=${AGAR_CFLAGS} ${AGAR_SG_CFLAGS} -I.
LIBS+=	${AGAR_LIBS} ${AGAR_SG_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
