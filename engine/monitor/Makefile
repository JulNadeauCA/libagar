#	$Csoft: Makefile,v 1.23 2004/04/22 01:52:24 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	pixmaps

LIB=		monitor
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	monitor.c widget_browser.c view_params.c screenshot.c uniconv.c \
	leak.c

CFLAGS+=${SDL_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
