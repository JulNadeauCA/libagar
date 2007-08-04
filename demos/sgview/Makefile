TOP=	../..
include Makefile.config

PROG=	sgview
SRCS=	sgview.c floor.c

CFLAGS+=${AGAR_CFLAGS} ${AGAR_SG_CFLAGS} ${AGAR_DEV_CFLAGS} -I.
LIBS+=	${AGAR_LIBS} ${AGAR_SG_LIBS} ${AGAR_DEV_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
