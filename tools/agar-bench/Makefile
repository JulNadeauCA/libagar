TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROG=		agar-bench
PROG_TYPE=	"GUI"
PROG_GUID=	"8CECE971-4D10-4787-B8CF-349F58861C95"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

SRCS=		agar-bench.c generic.c pixelops.c primitives.c surfaceops.c \
		memops.c misc.c events.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

release: cleandir
	sh mk/release.sh
	#sh mk/cvs.sh

.PHONY: configure release

include ${TOP}/mk/build.prog.mk
