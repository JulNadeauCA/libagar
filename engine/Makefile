#	$Csoft: Makefile,v 1.93 2005/04/14 06:19:35 vedge Exp $

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
	rg

LIB=		engine
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	audio.c config.c engine.c event.c gfx.c gobject.c input.c \
	mkpath.c object.c objmgr.c perso.c prop.c rcs.c space.c timeout.c \
	typesw.c view.c
MAN3=	engine.3 event.3 gfx.3 input.3 object.3 prop.3 timeout.3 view.3

CFLAGS+=${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
