#	$Csoft: Makefile,v 1.3 2005/06/18 04:25:21 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	pixmaps

LIB=		map
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	eraser.c fill.c flip.c invert.c magnifier.c map.c mapedit.c \
	mapview.c mediasel.c merge.c nodemask.c propedit.c \
	resize.c nodesel.c refsel.c shift.c stamp.c tool.c transform.c
MAN3=	map.3 mapview.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
