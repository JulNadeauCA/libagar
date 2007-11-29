TOP=	../..
include Makefile.config

PROJECT=	"maximized"
PROG_TYPE=	"GUI"
PROG_GUID=	"E444ED68-9880-4f2e-B0DF-46D6FC2AE83A"
PROG_LINKS=	ag_core ag_gui pthreads SDL SDLmain opengl freetype

PROG=	maximized
SRCS=	maximized.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
