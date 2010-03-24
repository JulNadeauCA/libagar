TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"customeventloop"
PROG_TYPE=	"GUI"
PROG_GUID=	"f460a31c-b51e-4f0e-b673-778a8e476131"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	customeventloop
SRCS=	customeventloop.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
