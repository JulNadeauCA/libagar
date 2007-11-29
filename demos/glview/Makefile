TOP=	../..
include Makefile.config

PROJECT=	"glview"
PROJINCLUDES=	configure.lua
PROG_TYPE=	"GUI"
PROG_GUID=	"B22C8324-AB8E-417e-9459-9FA0698D45D7"
PROG_LINKS=	ag_core ag_gui \
		pthreads SDL SDLmain opengl glu freetype

PROG=		glview
SRCS=		glview.c

CFLAGS+=${AGAR_CFLAGS} ${OPENGL_CFLAGS} ${GLU_CFLAGS} -I.
LIBS+=	${AGAR_LIBS} ${OPENGL_LIBS} ${GLU_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
