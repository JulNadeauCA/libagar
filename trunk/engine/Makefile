#	$Csoft: Makefile,v 1.45 2002/11/14 07:49:05 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	compat mapedit widget monitor

SHARE=	README.engine \
	README.xcf

CFLAGS+=	${SDL_CFLAGS} ${SMPEG_CFLAGS} ${FREETYPE_CFLAGS} ${X11_CFLAGS}

MAN3=	agar.3 map.3 object.3 view.3 version.3 rootmap.3 \
	world.3 event.3
CATMAN3=agar.cat3 map.cat3 object.cat3 view.cat3 version.cat3 rootmap.cat3 \
	world.cat3 event.cat3

LIB=	engine

SRCS=	engine.c error.c event.c anim.c oldxcf.c ttf.c view.c physics.c \
	input.c object.c media.c audio.c world.c map.c rootmap.c \
	char.c version.c config.c prop.c

OBJS=	engine.o error.o event.o anim.o oldxcf.o ttf.o view.o physics.o \
	input.o object.o media.o audio.o world.o map.o rootmap.o \
	char.o version.o config.o prop.o

OBJS+=	mapedit/mapedit.o \
	monitor/monitor.o

OBJS+=	compat/setenv.o \
	compat/strlcat.o \
	compat/strlcpy.o \
	compat/strsep.o \
	compat/vasprintf.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
