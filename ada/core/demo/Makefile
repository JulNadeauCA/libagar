TOP=	../../..
include Makefile.config

PROG=		agar_ada_core_demo

SRCS=		agar_ada_core_demo.adb \
		myatexit.adb \
		myeventhandler.adb \
		animal.adb

CFLAGS+=	${AGAR_ADA_CORE_CFLAGS} ${AGAR_CORE_CFLAGS}
LIBS+=		${AGAR_ADA_CORE_LIBS} ${AGAR_CORE_LIBS}

include ${TOP}/mk/build.prog.mk

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure
