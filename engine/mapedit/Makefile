#	$Csoft: Makefile,v 1.10 2002/02/21 02:17:24 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	mapedit
SRCS=	mapedit.c command.c mouse.c joy.c
OBJS=	mapedit.o command.o mouse.o joy.o
FOBJS=	mapedit.fob
SHARE=	README

CFLAGS+=	${SDL_CFLAGS}

all:	mapedit.fob mapedit_offs.h

depend:	mapedit.fob

mapedit.fob mapedit_offs.h: mapedit.xcf
	${FOBJCOMP} -o mapedit.fob mapedit.xcf

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
