TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"focusing"
PROG_TYPE=	"GUI"
PROG_GUID=	"B8BAE87E-AD27-47db-B61F-3720BE8C8A77"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	focusing
SRCS=	focusing.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
