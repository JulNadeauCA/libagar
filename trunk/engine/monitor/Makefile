#	$Csoft: Makefile,v 1.20 2003/11/17 15:07:15 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=		monitor
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	monitor.c widget_browser.c view_params.c screenshot.c uniconv.c

XCF=		toolbar.xcf monitor.xcf
DEN=		monitor.den
DEN_DIR=	engine/monitor
DEN_NAME=	"Debug monitor icons"
DEN_AUTHOR=	"vedge@"
DEN_COPYRIGHT=	"2002, 2003 CubeSoft Communications, Inc."

CFLAGS+=${SDL_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/dencomp/Makefile.inc
