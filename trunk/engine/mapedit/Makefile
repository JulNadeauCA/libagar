#	$Csoft: Makefile,v 1.13 2002/06/06 10:15:25 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	mapedit
SRCS=	mapedit.c command.c config.c toolbar.c
# SRCS+= mouse.c joy.c
OBJS=	mapedit.o command.o config.o toolbar.o
# OBJS+= mouse.o joy.o
FOBJS=	mapedit.fob
SHARE=	README.mapedit

CFLAGS+=	${SDL_CFLAGS}

all:	mapedit.fob mapedit_offs.h

depend:	mapedit.fob

mapedit.fob mapedit_offs.h: mapedit.xcf
	${FOBJCOMP} -o mapedit.fob mapedit.xcf

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
