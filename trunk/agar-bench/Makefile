TOP=	..
include ./Makefile.config

PROG=	agar-bench
SRCS=	agar-bench.c generic.c pixelops.c primitives.c surfaceops.c memops.c \
	misc.c events.c sg.c
VERSION=1.0

CFLAGS+=${AGAR_CFLAGS} ${AGAR_SG_CFLAGS} -I. -DHAVE_RDTSC -DVERSION=\"${VERSION}\" -D_USE_AGAR_MATH
LIBS+=	${AGAR_LIBS} ${AGAR_SG_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

release: cleandir
	sh mk/release.sh
	#sh mk/cvs.sh

.PHONY: configure release

include ${TOP}/mk/build.prog.mk
