#	$Csoft: Makefile,v 1.12 2002/04/18 03:58:57 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	mapedit
SRCS=	mapedit.c command.c mouse.c joy.c config.c
OBJS=	mapedit.o command.o mouse.o joy.o config.o
FOBJS=	mapedit.fob
SHARE=	README.mapedit

CFLAGS+=	${SDL_CFLAGS}

all:	mapedit.fob mapedit_offs.h

depend:	mapedit.fob

mapedit.fob mapedit_offs.h: mapedit.xcf
	${FOBJCOMP} -o mapedit.fob mapedit.xcf

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
