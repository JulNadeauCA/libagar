TOP=	../..
include Makefile.config

PROG=	velcurve
SRCS=	voxelpath.c velcurve.c

CFLAGS+=${AGAR_SG_CFLAGS} ${AGAR_SC_CFLAGS} ${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_SG_LIBS} ${AGAR_SC_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
