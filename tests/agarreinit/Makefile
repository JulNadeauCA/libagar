TOP=	../..
include Makefile.config
include ../Makefile.proj
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"agarreinit"
PROG_TYPE=	"GUI"
PROG_GUID=	"26b7d250-458f-11e2-beb9-001cf09eb927"
PROG_LINKS=	${GUI_LINKS_STATIC} ${CORE_LINKS_STATIC}
CFLAGS+=	${AGAR_CFLAGS}
LIBS+=		${AGAR_LIBS}

PROG=	agarreinit
SRCS=	agarreinit.c

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
