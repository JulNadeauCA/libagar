#	$Csoft: Makefile,v 1.2 2002/12/17 06:48:49 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	media
SRCS=	art.c xcf.c
OBJS=	art.o xcf.o

MAN5=	xcf.5
CATMAN5=xcf.cat5

CFLAGS+=	${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
