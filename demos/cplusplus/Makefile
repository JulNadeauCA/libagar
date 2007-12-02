TOP=	../..
include Makefile.config

CC=c++
PROJECT=	"cplusplus"
PROJINCLUDES=	configure.lua
PROG_TYPE=	"GUI"
PROG_GUID=	""
PROG_LINKS=	ag_core ag_gui pthreads SDL SDLmain opengl freetype

PROG=	cplusplus
SRCS=	cplusplus.cpp

CXXFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
