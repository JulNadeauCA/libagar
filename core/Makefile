#	$Csoft: Makefile,v 1.97 2005/09/20 13:46:29 vedge Exp $

TOP=..
include ${TOP}/Makefile.config

SUBDIR=	monitor \
	loaders

LIB=		ag_core
LIB_SHARED=	Yes
LIB_STATIC=	No
SHARESRC=	core-icons.den

SRCS=	config.c core.c error.c event.c math.c \
	object.c objmgr.c prop.c rcs.c timeout.c typesw.c view.c
MAN3=	AG_Intro.3 AG_Core.3 AG_Event.3 AG_Object.3 AG_Prop.3 \
	AG_Timeout.3 AG_View.3

CFLAGS+=${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS} \
	${AGAR_NET_CFLAGS} ${PTHREADS_XOPEN_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
