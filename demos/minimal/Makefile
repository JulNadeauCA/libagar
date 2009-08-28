TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"minimal"
PROG_TYPE=	"GUI"
PROG_GUID=	"1ce3d47e-18df-4e21-8cea-12370df730cd"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	minimal
SRCS=	minimal.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
