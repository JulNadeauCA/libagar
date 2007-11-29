TOP=	../..
include Makefile.config

PROJECT=	"loader"
PROG_TYPE=	"GUI"
PROG_GUID=	"D1E36CAF-5EC8-45dc-9050-CCEC50B2F7E3"
PROG_LINKS=	ag_core ag_gui pthreads SDL SDLmain opengl freetype

PROG=	loader
SRCS=	loader.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
