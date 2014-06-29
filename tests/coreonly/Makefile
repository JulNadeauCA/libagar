TOP=	../..
include Makefile.config
include ../Makefile.proj
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"coreonly"
PROG_TYPE=	"CLI"
PROG_GUID=	"887187c0-ff38-11e3-9191-0800200c9a66"
PROG_LINKS=	${CORE_LINKS_STATIC}
CFLAGS+=	${AGAR_CORE_CFLAGS}
CXXFLAGS+=	${AGAR_CORE_CFLAGS}
LIBS+=		${AGAR_CORE_LIBS}

PROG=	coreonly
SRCS=	coreonly.c

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
