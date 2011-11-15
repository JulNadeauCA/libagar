TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"compositing"
PROG_TYPE=	"GUI"
PROG_GUID=	"5807dde2-0f2f-11e1-a388-002421b9ca67"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	compositing
SRCS=	compositing.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
