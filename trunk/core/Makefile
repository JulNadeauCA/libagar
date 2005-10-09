#	$Csoft: Makefile,v 1.97 2005/09/20 13:46:29 vedge Exp $

TOP=..
include ${TOP}/Makefile.config

SUBDIR=	monitor \
	loaders

LIB=		ag_core
LIB_SHARED=	Yes
LIB_STATIC=	No
SHARESRC=	core-icons.den

SRCS=	audio.c config.c core.c error.c event.c gfx.c mkpath.c object.c \
	objmgr.c prop.c rcs.c timeout.c typesw.c view.c
MAN3=	agar.3 core.3 event.3 gfx.3 object.3 prop.3 timeout.3 view.3

CFLAGS+=${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
