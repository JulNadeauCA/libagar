TOP=	../..
include Makefile.config

PROJECT=	"windows"
PROJINCLUDES=	configure.lua
PROG_TYPE=	"GUI"
PROG_GUID=	"7085f719-bbbf-46a0-88d0-a4cd2b31eb74"
PROG_LINKS=	ag_core ag_gui pthreads SDL SDLmain opengl freetype

PROG=	windows
SRCS=	windows.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
