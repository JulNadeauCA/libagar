#	$Csoft: Makefile,v 1.51 2003/07/26 12:35:39 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool mapview

LIB=	mapedit
SRCS=	layedit.c mapedit.c mapview.c nodeedit.c objedit.c
OBJS=	layedit.o mapedit.o mapview.o nodeedit.o objedit.o

DEN=		mapedit.den
DEN_DIR=	engine/mapedit
DEN_NAME=	"Map editor icons"
DEN_AUTHOR=	"vedge@"
DEN_COPYRIGHT=	"2001, 2002, 2003 CubeSoft Communications, Inc"
XCF=		mapedit.xcf toolbar.xcf

CFLAGS+=${SDL_CFLAGS} ${GETTEXT_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/dencomp/Makefile.inc
