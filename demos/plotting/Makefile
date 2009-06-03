TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/math/Makefile.inc

PROJECT=	"plotting"
PROG_TYPE=	"GUI"
PROG_GUID=	"19dbbfcd-9e00-46ed-9f5c-7934652a6b9b"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS} ${MATH_LINKS}

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
