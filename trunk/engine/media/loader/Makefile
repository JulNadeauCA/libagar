#	$Csoft: Makefile,v 1.5 2003/03/05 18:27:03 vedge Exp $

TOP=	../../..
include ${TOP}/Makefile.config

LIB=	loader
SRCS=	xcf.c ttf.c
OBJS=	xcf.o ttf.o

MAN5=	xcf.5
CATMAN5=xcf.cat5
PSMAN5=	xcf.ps5

CFLAGS+=${SDL_CFLAGS} ${FREETYPE_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
