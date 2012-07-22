TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"network"
PROG_TYPE=	"GUI"
PROG_GUID=	"a921b65e-cfd6-11e1-b41c-003067c3e01e"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	network
SRCS=	network.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
