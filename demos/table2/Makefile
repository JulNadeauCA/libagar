TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"table"
PROG_TYPE=	"GUI"
PROG_GUID=	"A95B56E7-00FD-4390-9D87-E69F2C101BDF"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	table
SRCS=	table.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
