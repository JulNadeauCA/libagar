#	$Csoft: Makefile,v 1.27 2002/06/09 08:55:43 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	mapedit widget

SHARE=	README.engine \
	README.xcf

CFLAGS+=	${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS}

MAN3=	char.3 agar.3 map.3 physics.3 object.3
CATMAN3=char.cat3 agar.cat3 map.cat3 physics.cat3 object.cat3

LIB=	engine

SRCS=	engine.c error.c event.c anim.c xcf.c view.c physics.c input.c \
	object.c audio.c video.c world.c map.c char.c version.c \
	config.c ailment.c

OBJS=	engine.o error.o event.o anim.o xcf.o view.o physics.o input.o \
	object.o audio.o video.o world.o map.o char.o version.o \
	config.o ailment.o \
	mapedit/mapedit.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
