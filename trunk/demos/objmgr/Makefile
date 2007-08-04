TOP=	../..
include Makefile.config

PROG=	objmgr
SRCS=	objmgr.c

CFLAGS+=${AGAR_CFLAGS} ${AGAR_DEV_CFLAGS} -I.
LIBS+=	${AGAR_LIBS} ${AGAR_DEV_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
