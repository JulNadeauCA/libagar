#	$Csoft: Makefile,v 1.5 2003/08/13 01:47:34 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	loader

SRCS=	netbuf.c integral.c real.c string.c version.c den.c xcf.c ttf.c

MAN3=	version.3
MAN5=	xcf.5 den.5

CFLAGS+=${SDL_CFLAGS} ${FREETYPE_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
