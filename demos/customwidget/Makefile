TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"customwidget"
PROG_TYPE=	"GUI"
PROG_GUID=	"5d140e81-5592-4aa7-9395-16604afae922"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	customwidget
SRCS=	customwidget.c mywidget.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
