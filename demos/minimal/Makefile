TOP=	../..
include Makefile.config

PROJECT=	"minimal"
PROJINCLUDES=	configure.lua
PROG_TYPE=	"GUI"
PROG_GUID=	"4bf79e00-b744-404a-b809-5cbed7d2afc1"
PROG_LINKS=	ag_core ag_gui pthreads SDL SDLmain opengl freetype

PROG=	minimal
SRCS=	minimal.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
