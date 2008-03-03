TOP=	../..
include Makefile.config

PROJECT=	"cplusplus"
PROJINCLUDES=	configure.lua
PROG_TYPE=	"GUI"
PROG_GUID=	"a6ddd844-8f63-429d-8a04-71d37666b2ae"
PROG_LINKS=	ag_core ag_gui pthreads SDL SDLmain opengl freetype

CC=c++

PROG=	cplusplus
SRCS=	cplusplus.cpp

CFLAGS+=   ${AGAR_CFLAGS}
CXXFLAGS+= ${AGAR_CFLAGS} -I.
LIBS+=     ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
