#	$Csoft: Makefile,v 1.36 2002/08/24 04:04:35 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	compat mapedit widget

SHARE=	README.engine \
	README.xcf

CFLAGS+=	${SDL_CFLAGS} ${SMPEG_CFLAGS} ${FREETYPE_CFLAGS} ${X11_CFLAGS}

MAN3=	agar.3 map.3 object.3 view.3
CATMAN3=agar.cat3 map.cat3 object.cat3 view.cat3

LIB=	engine

SRCS=	engine.c error.c event.c anim.c oldxcf.c ttf.c view.c physics.c \
	input.c object.c audio.c video.c world.c map.c rootmap.c char.c \
	version.c config.c ailment.c

OBJS=	engine.o error.o event.o anim.o oldxcf.o ttf.o view.o physics.o \
	input.o object.o audio.o video.o world.o map.o rootmap.o char.o \
	version.o config.o ailment.o

OBJS+=	mapedit/mapedit.o
OBJS+=	compat/setenv.o \
	compat/strlcat.o \
	compat/strlcpy.o \
	compat/strsep.o \
	compat/vasprintf.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
