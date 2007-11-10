TOP=	../..
include Makefile.config

PROG=	plot3
SRCS=	plot3.c

CFLAGS+=${AGAR_SG_CFLAGS} ${AGAR_CFLAGS} -I. -D_USE_AGAR_MATH
LIBS+=	${AGAR_SG_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
