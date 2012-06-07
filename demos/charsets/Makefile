TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"charsets"
PROG_TYPE=	"GUI"
PROG_GUID=	"05089157-af91-11e1-9977-003067c3e01e"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	charsets
SRCS=	charsets.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk

