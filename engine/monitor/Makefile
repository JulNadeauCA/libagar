#	$Csoft: Makefile,v 1.24 2004/04/22 01:53:22 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	pixmaps

LIB=		monitor
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	leak.c monitor.c screenshot.c timeouts.c uniconv.c view_params.c \
	widget_browser.c

CFLAGS+=${SDL_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
