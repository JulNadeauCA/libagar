#	$Csoft: Makefile,v 1.94 2005/05/01 00:12:12 vedge Exp $

TOP=..
include ${TOP}/Makefile.config

SUBDIR=	error \
	map \
	widget \
	monitor \
	unicode \
	loader \
	input \
	icons \
	vg \
	rg \
	space

LIB=		engine
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	audio.c config.c engine.c event.c gfx.c gobject.c input.c \
	mkpath.c object.c objmgr.c perso.c prop.c rcs.c timeout.c \
	typesw.c view.c
MAN3=	engine.3 event.3 gfx.3 input.3 object.3 prop.3 timeout.3 view.3

CFLAGS+=${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
