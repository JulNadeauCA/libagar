TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/dev/Makefile.inc

PROJECT=	"objprops"
PROG_TYPE=	"GUI"
PROG_GUID=	"49C33F82-EC4D-4993-BA20-F64F80994353"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS} ${DEV_LINKS}

PROG=	objprops
SRCS=	objprops.c

CFLAGS+=${AGAR_DEV_CFLAGS} ${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_DEV_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
