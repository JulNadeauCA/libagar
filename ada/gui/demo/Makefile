TOP=	../../..
include Makefile.config

PROG=		agar_ada_demo
MAN1=		agar_ada_demo.1

SRCS=		agar_ada_demo.adb \
		myeventhandler.adb

CFLAGS+=	${AGAR_ADA_CFLAGS} ${AGAR_ADA_CORE_CFLAGS} ${AGAR_CFLAGS}
LIBS+=		${AGAR_ADA_LIBS} ${AGAR_ADA_CORE_LIBS} ${AGAR_LIBS}

CLEANFILES=	output.png output-index.png

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
