TOP=	../..
include Makefile.config

PROJECT=	"plotting"
PROJINCLUDES=	configure.lua
PROG_TYPE=	"GUI"
PROG_GUID=	"19dbbfcd-9e00-46ed-9f5c-7934652a6b9b"
PROG_LINKS=	ag_core ag_gui ag_sc pthreads SDL SDLmain opengl freetype

PROG=	plotting
SRCS=	plotting.c

CFLAGS+=${AGAR_MATH_CFLAGS} ${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_MATH_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
