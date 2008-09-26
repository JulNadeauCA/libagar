TOP=	..
include Makefile.config

PROG=		agar-disasm
PROG_TYPE=	"CLI"
PROG_GUID=	"e685b12e-1313-486c-80c1-5195cc03a3dd"
PROG_LINKS=	ag_core

SRCS=	agar-disasm.c
MAN1=	agar-disasm.1

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
