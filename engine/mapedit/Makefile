#	$Csoft: Makefile,v 1.3 2002/01/27 12:47:10 vedge Exp $

TOP=	../..

LIB=	mapedit
SRCS=	mapedit.c command.c
OBJS=	mapedit.o command.o
FOBJS=	mapedit.fob

CFLAGS+=	${SDL_CFLAGS} ${GLIB_CFLAGS}

all:	mapedit.fob mapedit_offs.h

mapedit.fob mapedit_offs.h: mapedit.xcf
	${FOBJCOMP} -o mapedit.fob mapedit.xcf

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/Makefile.inc
