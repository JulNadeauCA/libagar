#	$Csoft: Makefile,v 1.1 2004/03/17 06:04:59 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=		vg
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	vg.c vg_circle.c vg_line.c vg_math.c vg_point.c vg_primitive.c \
	vg_text.c vg_vertex.c vgedit.c

#MAN3=	vg.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/dencomp/Makefile.inc
include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
