#	$Csoft: Makefile,v 1.14 2003/06/06 02:18:04 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	monitor
SRCS=	monitor.c widget_browser.c view_params.c screenshot.c
OBJS=	monitor.o widget_browser.o view_params.o screenshot.o

FOBJ=	monitor.fob
FOBDIR=	/engine/monitor
FOFFS=	monitor_offs.h
XCF=	toolbar.xcf monitor.xcf

CFLAGS+=${SDL_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
