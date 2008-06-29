TOP=	../..
include Makefile.config

PROJECT=	"customwidget"
PROJINCLUDES=	configure.lua
PROG_TYPE=	"GUI"
PROG_GUID=	"6799678f-1748-4f28-b21b-ac09e60c8510"
PROG_LINKS=	ag_core ag_gui pthreads SDL SDLmain opengl freetype

PROG=	customwidget
SRCS=	customwidget.c mywidget.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
