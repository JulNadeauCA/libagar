#	$Csoft: Makefile,v 1.76 2004/03/09 06:16:18 vedge Exp $

TOP=..
include ${TOP}/Makefile.config

SUBDIR=	error \
	mapedit \
	widget \
	monitor \
	unicode \
	loader \
	input

LIB=	engine

SRCS=	audio.c config.c engine.c event.c gfx.c input.c map.c mkpath.c \
	object.c perso.c position.c prop.c rootmap.c transform.c \
	typesw.c view.c

MAN3=	agar.3 map.3 object.3 view.3 rootmap.3 event.3 prop.3 input.3

CFLAGS+=${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
