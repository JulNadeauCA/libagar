#	$Csoft: Makefile,v 1.8 2004/04/30 07:00:23 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=		loader
LIB_SHARED=	Yes
LIB_STATIC=	Yes

SRCS=	color.c den.c integral.c netbuf.c real.c string.c ttf.c version.c \
	vertex.c xcf.c

MAN3=	version.3
MAN5=	xcf.5 den.5

CFLAGS+=${SDL_CFLAGS} ${FREETYPE_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
