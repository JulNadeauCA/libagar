#	$Csoft: Makefile,v 1.23 2002/06/24 18:43:15 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool

LIB=	mapedit
SRCS=	mapedit.c config.c fileops.c toolbar.c mapview.c mapwin.c edcursor.c \
	tilestack.c tileq.c objq.c
OBJS=	mapedit.o config.o fileops.o toolbar.o mapview.o mapwin.o edcursor.o \
	tilestack.o tileq.o objq.o
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
