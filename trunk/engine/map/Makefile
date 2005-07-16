#	$Csoft: Makefile,v 1.4 2005/07/10 15:41:57 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	pixmaps

LIB=		map
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	eraser.c fill.c flip.c invert.c magnifier.c map.c mapedit.c \
	mapview.c merge.c nodemask.c propedit.c \
	resize.c nodesel.c refsel.c shift.c stamp.c tool.c transform.c
MAN3=	map.3 mapview.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
