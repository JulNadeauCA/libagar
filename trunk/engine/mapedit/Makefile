#	$Csoft: Makefile,v 1.7 2002/02/15 06:46:14 vedge Exp $

TOP=	../..

LIB=	mapedit
SRCS=	mapedit.c command.c key.c mouse.c joy.c
OBJS=	mapedit.o command.o key.o mouse.o joy.o
FOBJS=	mapedit.fob

CFLAGS+=	${SDL_CFLAGS} -DDATADIR=\"${PREFIX}/share\"

all:	mapedit.fob mapedit_offs.h

mapedit.fob mapedit_offs.h: mapedit.xcf
	${FOBJCOMP} -o mapedit.fob mapedit.xcf

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/Makefile.inc
