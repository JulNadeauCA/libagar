#	$Csoft: Makefile,v 1.1.1.1 2002/09/01 09:00:48 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool

LIB=	monitor
SRCS=	monitor.c
OBJS=	monitor.o
FOBJS=	monitor.fob

#MAN3=	monitor.3
#CATMAN3=monitor.cat3

CFLAGS+=	${SDL_CFLAGS}

XCF=	toolbar.xcf

all:	monitor.fob monitor_offs.h

depend:	monitor.fob

monitor.fob monitor_offs.h: ${XCF}
	${FOBJCOMP} -o monitor.fob ${XCF}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
