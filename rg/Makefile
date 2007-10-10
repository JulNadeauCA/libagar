TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_rg
LIB_SHARED=	Yes
LIB_STATIC=	No
LIB_GUID=	"54223b00-f4c5-4b49-ab84-cd2576c7f8d6"

SRCS=	tileset.c tileview.c tile.c feature.c fill.c pixmap.c prim.c sketch.c \
	sketch_line.c sketch_circle.c sketch_polygon.c animation.c \
	sketchproj.c texture.c texsel.c transform.c animview.c
MAN3=	RG_Tileview.3

CFLAGS+=${SDL_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
