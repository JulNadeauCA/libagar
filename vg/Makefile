TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_vg
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"6d009edb-c66a-46e8-a1dd-b6f653db6327"

LIB_LINKS=	ag_core ag_gui

SRCS=	vg.c vg_block.c vg_circle.c vg_arc.c vg_line.c vg_mask.c \
	vg_origin.c vg_ortho.c vg_point.c vg_primitive.c vg_snap.c \
	vg_tables.c vg_text.c vg_polygon.c vg_view.c vg_tool.c

#MAN3=	vg.3

CFLAGS+=-I.. ${SDL_CFLAGS}

include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
