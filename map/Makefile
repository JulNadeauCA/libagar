TOP=..

include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/math/Makefile.inc

LIB=		ag_map
LIB_CURRENT=	7
LIB_REVISION=	0
LIB_AGE=	0
LIB_INSTALL=	Yes
LIB_GUID=	"da8b3181-d485-4ef0-a68a-b747f08b5745"
LIB_LINKS=	${AGMATH_LINKS_STATIC} ${GUI_LINKS_STATIC} \
		${CORE_LINKS_STATIC} ${MAP_LIBS}

MAN3=	MAP.3 MAP_View.3 \
	RG_Tileset.3 RG_Feature.3 RG_Pixmap.3 RG_Tile.3 RG_Tileview.3

SRCS=	fill.c flip.c invert.c map.c mapview.c nodesel.c tool.c insert.c \
	insert_obj.c eraser.c \
	rg_tileset.c rg_tileview.c rg_tile.c rg_feature.c rg_fill.c \
	rg_pixmap.c rg_prim.c rg_texture.c rg_texsel.c rg_transform.c \
	map_object.c map_tile.c map_img.c map_link.c

# SRCS+= rg_sketch.c rg_sketch_line.c rg_sketch_circle.c rg_sketch_polygon.c
#        rg_sketchproj.c

CFLAGS+=${AGMATH_CFLAGS} \
	${GUI_CFLAGS} \
	${CORE_CFLAGS} \
	${MAP_CFLAGS} \
	-D_AGAR_MAP_INTERNAL -D_AGAR_RG_INTERNAL

LIBS=	-L../math -lag_math ${AGMATH_LIBS} \
	-L../gui  -lag_gui  ${GUI_LIBS} \
	-L../core -lag_core ${CORE_LIBS} \
	${MAP_LIBS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
