TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"threads"
PROG_TYPE=	"GUI"
PROG_GUID=	"7bbc1bab-f1ec-479c-be08-c92b823d838c"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	threads
SRCS=	threads.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
