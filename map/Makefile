#	$Csoft: Makefile,v 1.9 2005/08/21 04:17:29 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_map
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	fill.c flip.c invert.c map.c mapedit.c mapview.c nodemask.c \
	nodesel.c refsel.c tool.c insert.c ginsert.c eraser.c actor.c
MAN3=	MAP.3 MAP_View.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
