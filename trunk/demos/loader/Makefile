TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"loader"
PROG_TYPE=	"GUI"
PROG_GUID=	"D1E36CAF-5EC8-45dc-9050-CCEC50B2F7E3"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	loader
SRCS=	loader.c

CFLAGS+=${AGAR_CFLAGS} ${SDL_IMAGE_CFLAGS} -I.
LIBS+=	${AGAR_LIBS} ${SDL_IMAGE_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
