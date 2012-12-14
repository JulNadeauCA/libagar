TOP=	../..
include Makefile.config
include ../Makefile.proj
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"agarsdl"
PROG_TYPE=	"GUI"
PROG_GUID=	"38724b49-4595-11e2-beb9-001cf09eb927"
PROG_LINKS=	${GUI_LINKS_STATIC} ${CORE_LINKS_STATIC} \
		${SDL_LIBS}
CFLAGS+=	${AGAR_CFLAGS} ${SDL_CFLAGS}
LIBS+=		${AGAR_LIBS} ${SDL_LIBS}

PROG=	agarsdl
SRCS=	agarsdl.c

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
