TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/rg/Makefile.inc
include ${TOP}/vg/Makefile.inc
include ${TOP}/math/Makefile.inc

PROJECT=	"cplusplus"
PROG_TYPE=	"GUI"
PROG_GUID=	"a6ddd844-8f63-429d-8a04-71d37666b2ae"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS} \
		${RG_LINKS} ${VG_LINKS} ${MATH_LINKS}

CC=c++

PROG=	cplusplus
SRCS=	cplusplus.cpp

AGARFLAGS= ${AGAR_CFLAGS} ${AGAR_MATH_CFLAGS} ${AGAR_VG_CFLAGS} ${AGAR_RG_CFLAGS}
CFLAGS+=   ${AGARFLAGS}
CXXFLAGS+= ${AGARFLAGS} -I.
LIBS+=     ${AGAR_LIBS} ${AGAR_MATH_LIBS} ${AGAR_VG_LIBS} ${AGAR_RG_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
