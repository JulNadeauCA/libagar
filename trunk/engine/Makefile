#	$Csoft: Makefile,v 1.69 2003/06/19 01:43:40 vedge Exp $

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

SRCS=	engine.c event.c view.c physics.c \
	input.c object.c map.c rootmap.c perso.c \
	config.c prop.c transform.c \
	typesw.c mkpath.c gfx.c

OBJS=	engine.o event.o view.o physics.o \
	input.o object.o map.o rootmap.o perso.o \
	config.o prop.o transform.o \
	typesw.o mkpath.o gfx.o

OBJS+=	mapedit/mapedit.o \
	monitor/monitor.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
