#	$Csoft: Makefile,v 1.90 2005/01/13 02:30:20 vedge Exp $

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
	vg \
	rg

LIB=		engine
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	audio.c config.c engine.c event.c gfx.c input.c map.c mkpath.c \
	nodemask.c object.c objmgr.c perso.c prop.c rootmap.c space.c \
	timeout.c transform.c typesw.c view.c
MAN3=	engine.3 event.3 gfx.3 input.3 map.3 object.3 prop.3 rootmap.3 \
	timeout.3 view.3

CFLAGS+=${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
