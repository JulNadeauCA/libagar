TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/au/Makefile.inc

PROJECT=	"audio"
PROG_TYPE=	"GUI"
PROG_GUID=	"dad75646-8a63-4c30-ad9e-8b07221cd0ad"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS} ${AU_LINKS}

PROG=	audio
SRCS=	audio.c

CFLAGS+=${AGAR_AU_CFLAGS} ${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_AU_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
