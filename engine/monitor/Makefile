#	$Csoft: Makefile,v 1.6 2002/11/26 07:19:11 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	monitor
SRCS=	monitor.c object_browser.c level_browser.c \
	widget_browser.c view_params.c
OBJS=	monitor.o object_browser.o level_browser.o \
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
