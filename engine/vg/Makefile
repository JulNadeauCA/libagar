#	$Csoft: Makefile,v 1.12 2004/11/25 02:49:01 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=		vg
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	vg.c vg_block.c vg_circle.c vg_ellipse.c vg_line.c vg_mask.c \
	vg_math.c vg_origin.c vg_ortho.c vg_point.c vg_primitive.c vg_snap.c \
	vg_tables.c vg_text.c drawing.c vg_polygon.c

#MAN3=	vg.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
