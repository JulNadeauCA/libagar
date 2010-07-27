TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"pane"
PROG_TYPE=	"GUI"
PROG_GUID=	"a134d377-3205-4ef9-9b55-839dfafa1668"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	pane
SRCS=	pane.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
