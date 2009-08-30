TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"modalwindowhandler"
PROG_TYPE=	"GUI"
PROG_GUID=	"78aa7282-dbf1-4e81-9352-b290fcf37306"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	modalwindowhandler
SRCS=	modalwindowhandler.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
