#	$Csoft: Makefile,v 1.44 2003/06/21 06:50:22 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool mapview

LIB=	mapedit
SRCS=	mapedit.c fileops.c mapview.c objq.c nodeedit.c layedit.c selops.c \
	objedit.c
OBJS=	mapedit.o fileops.o mapview.o objq.o nodeedit.o layedit.o selops.o \
	objedit.o

OBJS+=	tool/*.o

DEN=		mapedit.den
DEN_DIR=	engine/mapedit
DEN_NAME=	"Map editor icons"
DEN_AUTHOR=	"vedge@"
DEN_COPYRIGHT=	"2001, 2002, 2003 CubeSoft Communications, Inc"
XCF=		mapedit.xcf toolbar.xcf

MAN3=	mapedit.3 mapview.3
CATMAN3=mapedit.cat3 mapview.cat3
PSMAN3=	mapedit.ps3 mapview.ps3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/dencomp/Makefile.inc
