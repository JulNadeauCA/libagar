TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"imageloading"
PROG_TYPE=	"GUI"
PROG_GUID=	"50b1064f-6f71-44de-a0d9-a61e500c445a"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	imageloading
SRCS=	imageloading.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
