#	$Csoft: Makefile,v 1.97 2005/09/20 13:46:29 vedge Exp $

TOP=..
include ${TOP}/Makefile.config

SUBDIR=	map

LIB=		ag_game
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	game.c actor.c perso.c space.c
#MAN3=	actor.3 perso.3 space.3

CFLAGS+=${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
