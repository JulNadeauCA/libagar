#	$Csoft: Makefile,v 1.3 2002/12/22 09:44:51 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	media
SRCS=	art.c xcf.c ttf.c
OBJS=	art.o xcf.o ttf.o

MAN5=	xcf.5
CATMAN5=xcf.cat5

CFLAGS+=	${SDL_CFLAGS} ${FREETYPE_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
