TOP=	../..
include Makefile.config
include ../Makefile.proj
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/au/Makefile.inc

PROJECT=	"agaraudio"
PROG_TYPE=	"GUI"
PROG_GUID=	"2b81f358-45a2-11e2-beb9-001cf09eb927"
PROG_LINKS=	${AU_LINKS_STATIC} ${GUI_LINKS_STATIC} ${CORE_LINKS_STATIC}
CFLAGS+=	${AGAR_AU_CFLAGS} ${AGAR_CFLAGS}
LIBS+=		${AGAR_AU_LIBS} ${AGAR_LIBS}

PROG=	agaraudio
SRCS=	agaraudio.c

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
