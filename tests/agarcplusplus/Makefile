TOP=	../..
include Makefile.config
include ../Makefile.proj
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"agarcplusplus"
PROG_TYPE=	"GUI"
PROG_GUID=	"0422726e-458e-11e2-beb9-001cf09eb927"
PROG_LINKS=	${AGMATH_LINKS_STATIC} ${GUI_LINKS_STATIC} ${CORE_LINKS_STATIC}
CFLAGS+=	${AGAR_MATH_CFLAGS} ${AGAR_CFLAGS}
CXXFLAGS+=	${AGAR_MATH_CFLAGS} ${AGAR_CFLAGS}
LIBS+=		${AGAR_MATH_LIBS} ${AGAR_LIBS} -lstdc++

PROG=	agarcplusplus
SRCS=	agarcplusplus.cpp

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
