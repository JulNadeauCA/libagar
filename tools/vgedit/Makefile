TOP=	../..
include Makefile.config

PROJECT=	"vgedit"
PROG=		vgedit
PROG_TYPE=	"GUI"
PROG_GUID=	"08704f9d-b2d9-11e9-8487-ac9e17b8d45b"
PROG_LINKS=	ag_vg ag_gui ag_core

SRCS=		vgedit.c
MAN1=		vgedit.1
#DATAFILES=	vgedit.png

CFLAGS+=${AGAR_VG_CFLAGS} ${AGAR_CFLAGS}
LIBS+=	${AGAR_VG_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
