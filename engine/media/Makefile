#	$Csoft: Makefile,v 1.4 2003/03/02 00:45:24 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	media
SRCS=	art.c xcf.c ttf.c
OBJS=	art.o xcf.o ttf.o

MAN5=	xcf.5
CATMAN5=xcf.cat5
PSMAN5=	xcf.ps5

CFLAGS+=	${SDL_CFLAGS} ${FREETYPE_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
