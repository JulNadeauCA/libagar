#	$Csoft: Makefile,v 1.9 2005/05/16 10:35:14 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=		rg
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	tileset.c tileview.c tile.c feature.c fill.c pixmap.c prim.c sketch.c \
	sketch_line.c sketch_circle.c animation.c polygon.c sketchproj.c
MAN3=	tileview.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
