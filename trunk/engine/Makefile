#	$Csoft: Makefile,v 1.4 2002/01/26 20:17:34 vedge Exp $

TOP=	..
SUBDIR=	mapedit

CFLAGS+=	${GLIB_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS}

LIB=	engine
SRCS=	event.c object.c char.c world.c map.c xcf.c view.c audio.c video.c
OBJS=	event.o object.o char.o world.o map.o xcf.o view.o audio.o video.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/Makefile.inc
