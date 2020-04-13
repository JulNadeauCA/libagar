TOP=	../..
include Makefile.config

PROJECT=	"mapedit"
#SUBDIR=	po

PROG=		mapedit
PROG_TYPE=	"GUI"
PROG_GUID=	"064385e7-1b25-11ea-861c-ac9e17b8d45b"
PROG_LINKS=	ag_map ag_gui ag_core

SRCS=		mapedit.c
DATAFILES_SRC=	mapedit.bmp
MAN1=		mapedit.1

CFLAGS+=	${AGAR_MAP_CFLAGS} ${AGAR_CFLAGS}
LIBS+=		${AGAR_MAP_LIBS} ${AGAR_LIBS}

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
