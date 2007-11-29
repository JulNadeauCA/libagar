TOP=	../..
include Makefile.config

PROJECT=	"sockets"
PROG_TYPE=	"GUI"
PROG_GUID=	"9438EB01-3853-438d-AEE7-471BCD28CA8F"
PROG_LINKS=	ag_core ag_gui pthreads SDL SDLmain opengl freetype

PROG=	sockets
SRCS=	sockets.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
