TOP=	../..
include Makefile.config

PROJECT=	"agarib"
#SUBDIR=	po

PROG=		agarib
PROG_TYPE=	"GUI"
PROG_GUID=	"9bdbc31b-4cb1-11ed-9819-ac9e17b8d45b"
PROG_LINKS=	ag_gui ag_core

SRCS=		agarib.c
#DATAFILES_SRC=	agarib.bmp
MAN1=		agarib.1

CFLAGS+=	${AGAR_CFLAGS} ${GETTEXT_CFLAGS}
#CXXFLAGS+=	${AGAR_CFLAGS} ${GETTEXT_CFLAGS}
LIBS+=		${AGAR_LIBS} ${GETTEXT_LIBS}

all: all-subdir ${PROG}

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
