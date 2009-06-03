TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"textbox"
PROG_TYPE=	"GUI"
PROG_GUID=	"0D29CDD8-4AEE-4717-BC3B-AA090F2956BD"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	textbox
SRCS=	textbox.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
