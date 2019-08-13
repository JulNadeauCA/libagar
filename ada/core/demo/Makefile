TOP=	../../..
include Makefile.config

PROG=	agar_ada_core_demo
MAN1=	agar_ada_core_demo.1

SRCS=	agar_ada_core_demo.adb \
	myatexit.adb \
	myeventhandler.adb \
	animal.adb

CLEANFILES+=	Cow.obj Cow.obj.bak My_Parent.obj My_Parent.obj.bak

CFLAGS+=${AGAR_ADA_CORE_CFLAGS} ${AGAR_CORE_CFLAGS}
LIBS+=	${AGAR_ADA_CORE_LIBS} ${AGAR_CORE_LIBS}

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
