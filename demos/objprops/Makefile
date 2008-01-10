TOP=	../..
include Makefile.config

PROJECT=	"objprops"
PROJINCLUDES=	configure.lua
PROG_TYPE=	"GUI"
PROG_GUID=	"49C33F82-EC4D-4993-BA20-F64F80994353"
PROG_LINKS=	ag_core ag_gui ag_dev pthreads SDL SDLmain opengl freetype

PROG=	objprops
SRCS=	objprops.c

CFLAGS+=${AGAR_DEV_CFLAGS} ${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_DEV_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
