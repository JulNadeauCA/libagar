TOP=..

include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/vg/Makefile.inc

LIB=		ag_vg
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_INSTALL=	Yes
LIB_GUID=	"6d009edb-c66a-46e8-a1dd-b6f653db6327"
LIB_LINKS=	${CORE_LINKS_STATIC} ${GUI_LINKS_STATIC} ${VG_LINKS_STATIC}

MAN3=	VG.3 VG_Arc.3 VG_Circle.3 VG_Line.3 VG_Point.3 VG_Polygon.3 \
	VG_Text.3 VG_View.3

SRCS=	vg.c vg_circle.c vg_arc.c vg_line.c vg_point.c vg_snap.c \
	vg_tables.c vg_text.c vg_polygon.c vg_view.c vg_tool.c \
	vg_circle_tool.c vg_line_tool.c vg_point_tool.c vg_proximity_tool.c \
	vg_text_tool.c vg_arc_tool.c vg_polygon_tool.c vg_select_tool.c

CFLAGS+=${GUI_CFLAGS} \
	${CORE_CFLAGS} \
	${VG_CFLAGS} -D_AGAR_VG_INTERNAL

LIBS=	${ENABLE_GUI_LIBS} ${GUI_LIBS} \
	-L../core -lag_core ${CORE_LIBS} \
	${VG_LIBS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
