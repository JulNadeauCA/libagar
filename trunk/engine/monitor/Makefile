#	$Csoft: Makefile,v 1.1.1.1 2002/09/01 09:03:06 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	monitor
SRCS=	monitor.c sprite_browser.c object_browser.c
OBJS=	monitor.o sprite_browser.o object_browser.o
FOBJS=	monitor.fob
XCF=	toolbar.xcf

#MAN3=	monitor.3
#CATMAN3=monitor.cat3

CFLAGS+=${SDL_CFLAGS}

all:	monitor.fob monitor_offs.h

depend:	monitor.fob

monitor.fob monitor_offs.h: ${XCF}
	${FOBJCOMP} -o monitor.fob ${XCF}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
