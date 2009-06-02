TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc

PROG=		denuncomp
PROG_TYPE=	"CLI"
PROG_GUID=	"3B69DB91-66D5-4d11-B77E-D83A26372A7D"
PROG_LINKS=	${CORE_LINKS}

SRCS=	denuncomp.c
MAN1=	denuncomp.1

CFLAGS+=${AGAR_CORE_CFLAGS}
LIBS+=	${AGAR_CORE_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
