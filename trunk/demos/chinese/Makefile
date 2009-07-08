TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"chinese"
PROG_TYPE=	"GUI"
PROG_GUID=	"bec6ad96-3a21-4719-99c9-b24c2531cc65"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	chinese
SRCS=	chinese.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
