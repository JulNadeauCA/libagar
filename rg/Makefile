TOP=..

include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/math/Makefile.inc
include ${TOP}/rg/Makefile.inc

LIB=		ag_rg
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_INSTALL=	Yes
LIB_GUID=	"54223b00-f4c5-4b49-ab84-cd2576c7f8d6"
LIB_LINKS=	${AGMATH_LINKS_STATIC} ${GUI_LINKS_STATIC} ${CORE_LINKS_STATIC} \
		${RG_LIBS}

MAN3=	RG_Feature.3 RG_Pixmap.3 RG_Tile.3 RG.3 RG_Tileview.3

SRCS=	tileset.c tileview.c tile.c feature.c fill.c pixmap.c prim.c \
	texture.c texsel.c transform.c

# TODO
#SRCS+= sketch.c sketch_line.c sketch_circle.c sketch_polygon.c sketchproj.c

CFLAGS+=${AGMATH_CFLAGS} \
	${GUI_CFLAGS} \
	${CORE_CFLAGS} \
	${RG_CFLAGS} -D_AGAR_RG_INTERNAL

LIBS=	-L../math -lag_math ${AGMATH_LIBS} \
	-L../gui  -lag_gui  ${GUI_LIBS} \
	-L../core -lag_core ${CORE_LIBS} \
	${RG_LIBS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
