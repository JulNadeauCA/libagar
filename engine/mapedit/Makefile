#	$Csoft: Makefile,v 1.46 2003/06/25 00:25:52 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool mapview

LIB=	mapedit
SRCS=	fileops.c layedit.c mapedit.c mapview.c nodeedit.c \
	objedit.c objq.c selops.c
OBJS=	fileops.o layedit.o mapedit.o mapview.o nodeedit.o \
	objedit.o objq.o selops.o
OBJS+=	tool/*.o

DEN=		mapedit.den
DEN_DIR=	engine/mapedit
DEN_NAME=	"Map editor icons"
DEN_AUTHOR=	"vedge@"
DEN_COPYRIGHT=	"2001, 2002, 2003 CubeSoft Communications, Inc"
XCF=		mapedit.xcf toolbar.xcf

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/dencomp/Makefile.inc
