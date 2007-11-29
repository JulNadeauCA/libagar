TOP=	../..
include Makefile.config

PROJECT=	"fixedres"
PROG_TYPE=	"GUI"
PROG_GUID=	"35A9469C-FAB5-4993-BCCC-C70374898B29"
PROG_LINKS=	ag_core ag_gui pthreads SDL SDLmain opengl freetype

PROG=		fixedres
SRCS=		fixedres.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
