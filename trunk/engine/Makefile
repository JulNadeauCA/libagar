#	$Csoft: Makefile,v 1.61 2003/05/09 01:58:53 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	compat \
	mapedit \
	widget \
	monitor \
	media

CFLAGS+=${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS}

MAN3=	agar.3 map.3 object.3 view.3 version.3 rootmap.3 \
	world.3 event.3 physics.3 prop.3 input.3
CATMAN3=agar.cat3 map.cat3 object.cat3 view.cat3 version.cat3 rootmap.cat3 \
	world.cat3 event.cat3 physics.cat3 prop.cat3 input.cat3
PSMAN3=	agar.ps3 map.ps3 object.ps3 view.ps3 version.ps3 rootmap.ps3 \
	world.ps3 event.ps3 physics.ps3 prop.ps3 input.ps3

LIB=	engine

SRCS=	engine.c error.c event.c view.c physics.c \
	input.c object.c world.c map.c rootmap.c perso.c \
	version.c config.c prop.c transform.c types.c

OBJS=	engine.o error.o event.o view.o physics.o \
	input.o object.o world.o map.o rootmap.o perso.o \
	version.o config.o prop.o transform.o types.o

OBJS+=	mapedit/mapedit.o \
	monitor/monitor.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
