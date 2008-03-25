TOP=	../..
include Makefile.config

PROJECT=	"initvideosdl"
PROJINCLUDES=	configure.lua
PROG_TYPE=	"GUI"
PROG_GUID=	"114ad05c-321b-42ff-8917-a2aa2cc44419"
PROG_LINKS=	ag_core ag_gui ag_dev pthreads SDL SDLmain opengl freetype

PROG=	initvideosdl
SRCS=	initvideosdl.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
