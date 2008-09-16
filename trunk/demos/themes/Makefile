TOP=	../..
include Makefile.config

PROJECT=	"themes"
PROJINCLUDES=	configure.lua
PROG_TYPE=	"GUI"
PROG_GUID=	"11D6C9FF-522E-43ed-B3EB-92A2C636CCA7"
PROG_LINKS=	ag_core ag_gui ag_dev pthreads SDL SDLmain opengl freetype

PROG=	themes
SRCS=	themes.c mytheme.c

CFLAGS+=${AGAR_DEV_CFLAGS} ${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_DEV_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
