#	$Csoft: Makefile,v 1.7 2002/01/30 12:11:38 vedge Exp $

TOP=	..
SUBDIR=	mapedit

CFLAGS+=	${GLIB_CFLAGS} ${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS}

MAN3=	char.3 agar.3 map.3
CATMAN3=char.cat3 agar.cat3 map.cat3

LIB=	engine
SRCS=	engine.c event.c object.c char.c world.c map.c xcf.c view.c \
	audio.c video.c
OBJS=	engine.o event.o object.o char.o world.o map.o xcf.o view.o \
	audio.o video.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/Makefile.inc
