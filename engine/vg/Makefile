#	$Csoft: Makefile,v 1.5 2004/04/17 00:43:39 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=		vg
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	vg.c vgobj.c vg_circle.c vg_ellipse.c vg_line.c vg_math.c vg_origin.c \
	vg_point.c vg_primitive.c vg_snap.c vg_tables.c vg_text.c

#MAN3=	vg.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/dencomp/Makefile.inc
include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
