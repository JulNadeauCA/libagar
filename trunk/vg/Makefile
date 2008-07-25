TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_vg
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"6d009edb-c66a-46e8-a1dd-b6f653db6327"
LIB_LINKS=	ag_core_static ag_gui_static pthreads \
		SDL opengl freetype

SRCS=	vg.c vg_circle.c vg_arc.c vg_line.c vg_ortho.c vg_point.c vg_snap.c \
	vg_tables.c vg_text.c vg_polygon.c vg_view.c vg_tool.c \
	vg_circle_tool.c vg_line_tool.c vg_point_tool.c vg_proximity_tool.c \
	vg_text_tool.c vg_arc_tool.c

MAN3=	VG.3 VG_Arc.3 VG_Circle.3 VG_Line.3 VG_Point.3 VG_Polygon.3 VG_Text.3 \
	VG_View.3

CFLAGS+=-I.. ${SDL_CFLAGS} ${MATH_CFLAGS} ${GETTEXT_CFLAGS}

include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
