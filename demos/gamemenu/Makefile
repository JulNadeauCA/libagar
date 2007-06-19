TOP=	../..
include Makefile.config

PROG=	gamemenu
SRCS=	gamemenu.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | manuconf > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
