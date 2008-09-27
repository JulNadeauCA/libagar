TOP=	../..
include Makefile.config

PROJECT=	"threads"
PROJINCLUDES=	configure.lua
PROG_TYPE=	"GUI"
PROG_GUID=	"7bbc1bab-f1ec-479c-be08-c92b823d838c"
PROG_LINKS=	ag_core ag_gui pthreads SDL SDLmain opengl freetype

PROG=	threads
SRCS=	threads.c

CFLAGS+=${AGAR_CFLAGS} ${SDL_IMAGE_CFLAGS} -I.
LIBS+=	${AGAR_LIBS} ${SDL_IMAGE_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
