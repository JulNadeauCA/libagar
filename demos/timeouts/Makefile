TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"timeouts"
PROG_TYPE=	"GUI"
PROG_GUID=	"fd1aa709-d317-4865-8112-ee1cab8d797a"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	timeouts
SRCS=	timeouts.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
