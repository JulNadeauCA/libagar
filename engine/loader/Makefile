#	$Csoft: Makefile,v 1.1 2003/06/19 01:53:38 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	loader
SRCS=	netbuf.c integral.c real.c string.c unicode.c version.c \
	den.c xcf.c ttf.c
OBJS=	netbuf.o integral.o real.o string.o unicode.o version.o \
	den.o xcf.o ttf.o

MAN3=	version.3
CATMAN3=version.cat3
PSMAN3=	version.ps3

MAN5=	xcf.5 den.5
CATMAN5=xcf.cat5 den.cat5
PSMAN5=	xcf.ps5 den.ps5

CFLAGS+=${SDL_CFLAGS} ${FREETYPE_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
