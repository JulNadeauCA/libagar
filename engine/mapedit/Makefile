#	$Csoft: Makefile,v 1.56 2003/09/06 23:05:51 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool \
	mapview

LIB=	mapedit
SRCS=	layedit.c mapedit.c mapview.c nodeedit.c objedit.c
MAN3=	mapview.3

XCF=		toolbar.xcf
DEN=		mapedit.den
DEN_DIR=	engine/mapedit
DEN_NAME=	"Map edition tool icons"
DEN_AUTHOR=	"vedge@"
DEN_COPYRIGHT=	"2001, 2002, 2003 CubeSoft Communications, Inc"

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/dencomp/Makefile.inc
