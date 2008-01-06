TOP=	../..
include Makefile.config

PROJECT=	"textbox"
PROJINCLUDES=	configure.lua
PROG_TYPE=	"GUI"
PROG_GUID=	"0D29CDD8-4AEE-4717-BC3B-AA090F2956BD"
PROG_LINKS=	ag_core ag_gui pthreads SDL SDLmain opengl freetype

PROG=	textbox
SRCS=	textbox.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
