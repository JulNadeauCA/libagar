#	$Csoft: Makefile,v 1.70 2003/06/21 06:50:18 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	compat \
	error \
	mapedit \
	widget \
	monitor \
	unicode \
	loader

CFLAGS+=${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS}

MAN3=	agar.3 map.3 object.3 \
	view.3 rootmap.3 \
	event.3 physics.3 prop.3 \
	input.3
CATMAN3=agar.cat3 map.cat3 object.cat3 \
	view.cat3 rootmap.cat3 \
	event.cat3 physics.cat3 prop.cat3 \
	input.cat3
PSMAN3=	agar.ps3 map.ps3 object.ps3 \
	view.ps3 rootmap.ps3 \
	event.ps3 physics.ps3 prop.ps3 \
	input.ps3

LIB=	engine

SRCS=	audio.c config.c engine.c event.c gfx.c input.c map.c \
	mkpath.c object.c perso.c physics.c prop.c rootmap.c \
	transform.c typesw.c view.c
OBJS=	audio.o config.o engine.o event.o gfx.o input.o map.o \
	mkpath.o object.o perso.o physics.o prop.o rootmap.o \
	transform.o typesw.o view.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
