#	$Csoft: Makefile,v 1.22 2004/03/18 21:27:48 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=		monitor
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	monitor.c widget_browser.c view_params.c screenshot.c uniconv.c \
	leak.c

CFLAGS+=${SDL_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
