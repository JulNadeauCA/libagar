#	$Csoft: Makefile,v 1.9 2002/12/24 10:32:41 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	monitor
SRCS=	monitor.c object_browser.c level_browser.c widget_browser.c \
	view_params.c screenshot.c
OBJS=	monitor.o object_browser.o level_browser.o widget_browser.o \
	view_params.o screenshot.o

FOBJS=	monitor.fob
XCF=	toolbar.xcf monitor.xcf

CFLAGS+=${SDL_CFLAGS} ${JPEG_CFLAGS}

all:	monitor.fob monitor_offs.h

depend:	monitor.fob

monitor.fob monitor_offs.h: ${XCF}
	${FOBJCOMP} -o monitor.fob ${XCF}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
