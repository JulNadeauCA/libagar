TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"textdlg"
PROG_TYPE=	"GUI"
PROG_GUID=	"4bf79e00-b744-404a-b809-5cbed7d2afc1"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	textdlg
SRCS=	textdlg.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
