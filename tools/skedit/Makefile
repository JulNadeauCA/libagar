TOP=	../..
include Makefile.config

PROJECT=	"skedit"
PROG=		skedit
PROG_TYPE=	"GUI"
PROG_GUID=	"d8c4f2fb-94aa-416e-abec-f5937c028c4c"
PROG_LINKS=	ag_sk ag_gui ag_core

SRCS=		skedit.c
MAN1=		skedit.1
#DATAFILES=	skedit.png

CFLAGS+=	${AGAR_SK_CFLAGS} ${AGAR_MATH_CFLAGS} ${AGAR_CFLAGS}
LIBS+=		${AGAR_SK_LIBS} ${AGAR_MATH_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
