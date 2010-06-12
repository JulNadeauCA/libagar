TOP=	..
include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/vg/Makefile.inc

LIB=		ag_rg
LIB_MAJOR=	4
LIB_MINOR=	0
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"54223b00-f4c5-4b49-ab84-cd2576c7f8d6"
LIB_LINKS=	${CORE_LINKS_STATIC} ${GUI_LINKS_STATIC} ${VG_LINKS_STATIC}

SRCS=	tileset.c tileview.c tile.c feature.c fill.c pixmap.c prim.c \
	animation.c texture.c texsel.c transform.c animview.c
MAN3=	RG_Anim.3 RG_Feature.3 RG_Pixmap.3 RG_Tile.3 RG.3 RG_Tileview.3

#SRCS+= sketch.c sketch_line.c sketch_circle.c sketch_polygon.c sketchproj.c

CFLAGS+=-D_AGAR_RG_INTERNAL ${SDL_CFLAGS} ${MATH_CFLAGS} ${GETTEXT_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
