#	$Csoft: Makefile,v 1.5 2005/03/03 10:51:01 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=		rg
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	tileset.c tileview.c tile.c feature.c fill.c pixmap.c prim.c sketch.c \
	sketch_line.c sketch_circle.c
#MAN3=	tileset.3 tileview.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
