TOP=	../..
include Makefile.config

PROG=	plot2
SRCS=	voxelpath.c plot2.c

CFLAGS+=${AGAR_SG_CFLAGS} ${AGAR_SC_CFLAGS} ${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_SG_LIBS} ${AGAR_SC_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
