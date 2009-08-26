TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"rendertosurface"
PROG_TYPE=	"GUI"
PROG_GUID=	"ea1d0404-9567-47ab-a531-f6be0929ddfb"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	rendertosurface
SRCS=	rendertosurface.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
