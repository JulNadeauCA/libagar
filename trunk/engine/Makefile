#	$Csoft: Makefile,v 1.51 2002/12/01 10:39:09 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	compat \
	mapedit \
	widget \
	monitor \
	media

SHARE=	README.engine \
	README.xcf

CFLAGS+=	${SDL_CFLAGS} ${SMPEG_CFLAGS} ${FREETYPE_CFLAGS} ${X11_CFLAGS}

MAN3=	agar.3 map.3 object.3 view.3 version.3 rootmap.3 \
	world.3 event.3 physics.3
CATMAN3=agar.cat3 map.cat3 object.cat3 view.cat3 version.cat3 rootmap.cat3 \
	world.cat3 event.cat3 physics.cat3

LIB=	engine

SRCS=	engine.c error.c event.c oldxcf.c ttf.c view.c physics.c \
	input.c object.c audio.c world.c map.c rootmap.c perso.c \
	version.c config.c prop.c transform.c

OBJS=	engine.o error.o event.o oldxcf.o ttf.o view.o physics.o \
	input.o object.o audio.o world.o map.o rootmap.o perso.o \
	version.o config.o prop.o transform.o

OBJS+=	mapedit/mapedit.o \
	monitor/monitor.o \
	media/art.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
