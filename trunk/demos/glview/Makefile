TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"glview"
PROG_TYPE=	"GUI"
PROG_GUID=	"B22C8324-AB8E-417e-9459-9FA0698D45D7"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS} opengl glu

PROG=		glview
SRCS=		glview.c

CFLAGS+=${AGAR_CFLAGS} ${OPENGL_CFLAGS} ${MATH_CFLAGS} -I.
LIBS+=	${AGAR_LIBS} ${OPENGL_LIBS} ${MATH_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
