#	$Csoft: Makefile,v 1.3 2005/02/08 15:50:29 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=		rg
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	tileset.c tileview.c tile.c feature.c fill.c pixmap.c prim.c
#MAN3=	tileset.3 tileview.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
