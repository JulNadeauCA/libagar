#	$Csoft: Makefile,v 1.1 2003/06/18 01:12:28 vedge Exp $

TOP=	../../..
include ${TOP}/Makefile.config

LIB=	loader
SRCS=	integral.c real.c string.c unicode.c xcf.c ttf.c version.c
OBJS=	integral.o real.o string.o unicode.o xcf.o ttf.o version.o

MAN3=	version.3
CATMAN3=version.cat3
PSMAN3=	version.ps3

MAN5=	xcf.5
CATMAN5=xcf.cat5
PSMAN5=	xcf.ps5

CFLAGS+=${SDL_CFLAGS} ${FREETYPE_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
