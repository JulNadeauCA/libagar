#	$Csoft: Makefile,v 1.60 2004/03/10 16:58:39 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=		vg
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	vg.c \
	vg_math.c \
	vg_points.c \
	vg_text.c

#MAN3=	vg.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/dencomp/Makefile.inc
include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
