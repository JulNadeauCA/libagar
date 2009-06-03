TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"scrollview"
PROG_TYPE=	"GUI"
PROG_GUID=	"831bd651-cd1d-438a-82bd-27a304ab2971"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	scrollview
SRCS=	scrollview.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
