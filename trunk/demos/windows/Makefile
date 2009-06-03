TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"windows"
PROG_TYPE=	"GUI"
PROG_GUID=	"7085f719-bbbf-46a0-88d0-a4cd2b31eb74"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	windows
SRCS=	windows.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
