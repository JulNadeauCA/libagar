#	$Csoft: Makefile,v 1.4 2002/02/10 01:36:37 vedge Exp $

TOP=	../..

LIB=	mapedit
SRCS=	mapedit.c command.c mouse.c
OBJS=	mapedit.o command.o mouse.o
FOBJS=	mapedit.fob

CFLAGS+=	${SDL_CFLAGS} ${GLIB_CFLAGS}

all:	mapedit.fob mapedit_offs.h

mapedit.fob mapedit_offs.h: mapedit.xcf
	${FOBJCOMP} -o mapedit.fob mapedit.xcf

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/Makefile.inc
