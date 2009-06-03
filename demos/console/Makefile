TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"console"
PROG_TYPE=	"GUI"
PROG_GUID=	"f6a4c917-dcce-46a8-a5e7-3094dcb73174"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	console
SRCS=	console.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
