#	$Csoft: Makefile,v 1.20 2002/06/23 02:40:33 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	mapedit
SRCS=	mapedit.c config.c fileops.c toolbar.c mapview.c mapwin.c edcursor.c \
	tilewin.c tilestack.c
OBJS=	mapedit.o config.o fileops.o toolbar.o mapview.o mapwin.o edcursor.o \
	tilewin.o tilestack.o
FOBJS=	mapedit.fob edcursor.fob

CFLAGS+=	${SDL_CFLAGS}

all:	mapedit.fob mapedit_offs.h edcursor.fob edcursor_offs.h

depend:	mapedit.fob edcursor.fob

mapedit.fob mapedit_offs.h: mapedit.xcf toolbar.xcf
	${FOBJCOMP} -o mapedit.fob mapedit.xcf toolbar.xcf

edcursor.fob edcursor_offs.h: edcursor.xcf
	${FOBJCOMP} -o edcursor.fob edcursor.xcf

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
