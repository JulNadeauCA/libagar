#	$Csoft: Makefile,v 1.6 2002/02/10 04:33:13 vedge Exp $

TOP=	../..

LIB=	mapedit
SRCS=	mapedit.c command.c key.c mouse.c joy.c
OBJS=	mapedit.o command.o key.o mouse.o joy.o
FOBJS=	mapedit.fob

CFLAGS+=	${SDL_CFLAGS} ${GLIB_CFLAGS}

all:	mapedit.fob mapedit_offs.h

mapedit.fob mapedit_offs.h: mapedit.xcf
	${FOBJCOMP} -o mapedit.fob mapedit.xcf

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/Makefile.inc
