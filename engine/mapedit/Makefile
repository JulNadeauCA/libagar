#	$Csoft: Makefile,v 1.14 2002/06/20 00:40:14 vedge Exp $

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

mapedit.fob mapedit_offs.h: mapedit.xcf toolbar.xcf
	${FOBJCOMP} -o mapedit.fob mapedit.xcf toolbar.xcf

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
