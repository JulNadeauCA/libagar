#	$Csoft: Makefile,v 1.7 2005/07/24 06:55:57 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	pixmaps

LIB=		map
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	fill.c flip.c invert.c map.c mapedit.c mapview.c nodemask.c \
	nodesel.c refsel.c tool.c transform.c insert.c ginsert.c
MAN3=	map.3 mapview.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
