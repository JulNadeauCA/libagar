#	$Csoft: Makefile,v 1.22 2002/04/18 04:03:50 vedge Exp $


TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	mapedit widget

SHARE=	README.engine

CFLAGS+=	${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS}

MAN3=	char.3 agar.3 map.3 physics.3 object.3
CATMAN3=char.cat3 agar.cat3 map.cat3 physics.cat3 object.cat3

LIB=	engine

SRCS=	engine.c event.c anim.c xcf.c view.c physics.c input.c \
	object.c audio.c video.c world.c map.c char.c version.c

OBJS=	engine.o event.o anim.o xcf.o view.o physics.o input.o \
	object.o audio.o video.o world.o map.o char.o version.o \
	mapedit/mapedit.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
