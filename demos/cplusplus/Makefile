TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"cplusplus"
PROG_TYPE=	"GUI"
PROG_GUID=	"a6ddd844-8f63-429d-8a04-71d37666b2ae"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

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
