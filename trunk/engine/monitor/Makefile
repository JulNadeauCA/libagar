#	$Csoft: Makefile,v 1.12 2003/05/14 03:43:31 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	monitor
SRCS=	monitor.c object_browser.c widget_browser.c view_params.c screenshot.c
OBJS=	monitor.o object_browser.o widget_browser.o view_params.o screenshot.o

FOBJ=	monitor.fob
FOFFS=	monitor_offs.h
XCF=	toolbar.xcf monitor.xcf

CFLAGS+=${SDL_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
