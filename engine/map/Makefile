#	$Csoft: Makefile,v 1.2 2005/06/11 11:13:31 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	pixmaps

LIB=		map
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	eraser.c fill.c flip.c invert.c magnifier.c map.c mapedit.c \
	mapview.c mediasel.c merge.c nodemask.c propedit.c \
	resize.c select.c shift.c stamp.c tool.c transform.c
MAN3=	map.3 mapview.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
