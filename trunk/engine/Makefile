#	$Csoft: Makefile,v 1.81 2004/04/11 03:29:15 vedge Exp $

TOP=..
include ${TOP}/Makefile.config

SUBDIR=	error \
	mapedit \
	widget \
	monitor \
	unicode \
	loader \
	input \
	icons \
	vg

LIB=		engine
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	audio.c config.c engine.c event.c gfx.c input.c map.c mkpath.c \
	nodemask.c object.c perso.c position.c prop.c rootmap.c timeout.c \
	transform.c typesw.c view.c
MAN3=	agar.3 event.3 input.3 map.3 object.3 prop.3 rootmap.3 timeout.3 view.3

CFLAGS+=${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
