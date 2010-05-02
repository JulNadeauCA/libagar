TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"sdl"
PROG_TYPE=	"GUI"
PROG_GUID=	"ca2f582f-9693-4d6f-a625-ed2d2d53c9eb"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	sdl
SRCS=	sdl.c

CFLAGS+=${AGAR_CFLAGS} ${SDL_IMAGE_CFLAGS}
LIBS+=	${AGAR_LIBS} ${SDL_IMAGE_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
