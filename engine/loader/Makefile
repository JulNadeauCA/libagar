#	$Csoft: Makefile,v 1.6 2003/08/31 11:58:08 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=		loader
LIB_SHARED=	Yes
LIB_STATIC=	Yes

SRCS=	netbuf.c integral.c real.c string.c version.c den.c xcf.c ttf.c

MAN3=	version.3
MAN5=	xcf.5 den.5

CFLAGS+=${SDL_CFLAGS} ${FREETYPE_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
