TOP=	../..
include Makefile.config

PROJECT=	"sgedit"
PROG=		sgedit
PROG_TYPE=	"GUI"
PROG_GUID=	"97cc7cff-f0a3-4176-b42c-b767352dd980"
PROG_LINKS=	ag_sg ag_gui ag_core

SRCS=		sgedit.c
MAN1=		sgedit.1
#DATAFILES=	sgedit.png

CFLAGS+=${AGAR_SG_CFLAGS} ${AGAR_MATH_CFLAGS} ${AGAR_CFLAGS}
LIBS+=	${AGAR_SG_LIBS} ${AGAR_MATH_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
