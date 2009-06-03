TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"unitconv"
PROG_TYPE=	"GUI"
PROG_GUID=	"0AFF01B9-205B-4f42-8619-69BF8B58C619"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	unitconv
SRCS=	unitconv.c

CFLAGS+=${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
