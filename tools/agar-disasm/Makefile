TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"agar-disasm"
PROG=		agar-disasm
PROG_TYPE=	"CLI"
PROG_GUID=	"e685b12e-1313-486c-80c1-5195cc03a3dd"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

SRCS=	agar-disasm.c
MAN1=	agar-disasm.1

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
