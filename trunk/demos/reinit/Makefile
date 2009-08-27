TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"reinit"
PROG_TYPE=	"GUI"
PROG_GUID=	"276f44e2-d725-40ac-9555-f1101939743b"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	reinit
SRCS=	reinit.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
