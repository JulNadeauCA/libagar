TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"scrollbar"
PROG_TYPE=	"GUI"
PROG_GUID=	"6799678f-1748-4f28-b21b-ac09e60c8510"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	scrollbar
SRCS=	scrollbar.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
