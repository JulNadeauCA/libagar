#	$Csoft: Makefile,v 1.25 2004/05/12 05:34:13 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	pixmaps

LIB=		monitor
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	leak.c monitor.c screenshot.c server.c timeouts.c uniconv.c \
	view_params.c widget_browser.c

CFLAGS+=${SDL_CFLAGS} ${JPEG_CFLAGS} ${LIBQNET_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
