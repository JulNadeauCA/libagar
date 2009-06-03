TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"keyevents"
PROG_TYPE=	"GUI"
PROG_GUID=	"cfde128b-ef9c-42c5-a571-dc8dd6d2650e"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	keyevents
SRCS=	keyevents.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
