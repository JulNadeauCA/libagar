#	$Csoft: Makefile,v 1.5 2002/11/16 01:36:58 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	monitor
SRCS=	monitor.c sprite_browser.c object_browser.c level_browser.c \
	widget_browser.c view_params.c
OBJS=	monitor.o sprite_browser.o object_browser.o level_browser.o \
	widget_browser.o view_params.o

FOBJS=	monitor.fob
XCF=	toolbar.xcf

CFLAGS+=${SDL_CFLAGS}

all:	monitor.fob monitor_offs.h

depend:	monitor.fob

monitor.fob monitor_offs.h: ${XCF}
	${FOBJCOMP} -o monitor.fob ${XCF}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
