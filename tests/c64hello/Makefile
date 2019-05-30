TOP=../..
include Makefile.config

PROJECT=	"c64-test"
PROG_TYPE=	"GUI"
PROG_GUID=	"f6ece979-70a4-11e9-a96a-ac9e17b8d45b"

PROG=		"c64hello"
SRCS=		hello.c

CFLAGS+=	${AGAR_CORE_CFLAGS}
LIBS=		${AGAR_CORE_LIBS}

all: ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
