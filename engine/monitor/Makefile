#	$Csoft: Makefile,v 1.16 2003/06/21 06:50:25 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	monitor
SRCS=	monitor.c widget_browser.c view_params.c screenshot.c
OBJS=	monitor.o widget_browser.o view_params.o screenshot.o

DEN=		monitor.den
DEN_DIR=	engine/monitor
DEN_NAME=	"Debug monitor icons"
DEN_AUTHOR=	"vedge@"
DEN_COPYRIGHT=	"2002, 2003 CubeSoft Communications, Inc."
XCF=		toolbar.xcf monitor.xcf

CFLAGS+=${SDL_CFLAGS} ${JPEG_CFLAGS} ${GETTEXT_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/dencomp/Makefile.inc
