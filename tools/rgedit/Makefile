TOP=	../..
include Makefile.config

PROJECT=	"rgedit"
#SUBDIR=	po

PROG=		rgedit
PROG_TYPE=	"GUI"
PROG_GUID=	"1a174c77-64f7-11e2-b702-001cf09eb927"
PROG_LINKS=	ag_rg ag_gui ag_core

SRCS=		rgedit.c
DATAFILES_SRC=	rgedit.bmp
MAN1=		rgedit.1

CFLAGS+=	${AGAR_MAP_CFLAGS} ${AGAR_CFLAGS}
LIBS+=		${AGAR_MAP_LIBS} ${AGAR_LIBS}

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
