#	$Csoft: Makefile,v 1.53 2003/08/06 04:11:28 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool \
	mapview

LIB=	mapedit
SRCS=	layedit.c mapedit.c mapview.c nodeedit.c objedit.c

XCF=		mapedit.xcf toolbar.xcf
DEN=		mapedit.den
DEN_DIR=	engine/mapedit
DEN_NAME=	"Map editor icons"
DEN_AUTHOR=	"vedge@"
DEN_COPYRIGHT=	"2001, 2002, 2003 CubeSoft Communications, Inc"

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/dencomp/Makefile.inc
