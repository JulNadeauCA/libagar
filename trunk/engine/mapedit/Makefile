#	$Csoft: Makefile,v 1.16 2002/06/22 16:03:04 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	mapedit
SRCS=	mapedit.c config.c fileops.c toolbar.c mapview.c mapwin.c
OBJS=	mapedit.o config.o fileops.o toolbar.o mapview.o mapwin.o
# command
FOBJS=	mapedit.fob edcursor.fob
SHARE=	README.mapedit

CFLAGS+=	${SDL_CFLAGS}

all:	mapedit.fob mapedit_offs.h edcursor.fob edcursor_offs.h

depend:	mapedit.fob edcursor.fob

mapedit.fob mapedit_offs.h: mapedit.xcf toolbar.xcf
	${FOBJCOMP} -o mapedit.fob mapedit.xcf toolbar.xcf

edcursor.fob edcursor_offs.h: edcursor.xcf
	${FOBJCOMP} -o edcursor.fob edcursor.xcf

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
