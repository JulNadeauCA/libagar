#	$Csoft: Makefile,v 1.62 2004/03/31 00:24:09 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool \
	mapview

LIB=		mapedit
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	layedit.c mapedit.c mapview.c mediasel.c nodeedit.c objedit.c tool.c
MAN3=	mapview.3

XCF=		toolbar.xcf \
		cursors.xcf \
		objedit.xcf \
		vg.xcf

DEN=		mapedit.den
DEN_DIR=	engine/mapedit
DEN_NAME=	"Level editor graphics"
DEN_AUTHOR=	"vedge@"
DEN_COPYRIGHT=	"2001, 2002, 2003 CubeSoft Communications, Inc"

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/dencomp/Makefile.inc
