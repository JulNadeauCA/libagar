TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"maximized"
PROG_TYPE=	"GUI"
PROG_GUID=	"E444ED68-9880-4f2e-B0DF-46D6FC2AE83A"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	maximized
SRCS=	maximized.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
