#	$Csoft: Makefile,v 1.42 2003/05/20 11:26:01 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool mapview

LIB=	mapedit
SRCS=	mapedit.c fileops.c mapview.c objq.c nodeedit.c layedit.c selops.c \
	objedit.c
OBJS=	mapedit.o fileops.o mapview.o objq.o nodeedit.o layedit.o selops.o \
	objedit.o

FOBJ=	mapedit.fob
FOBDIR=	/engine/mapedit
FOFFS=	mapedit_offs.h
XCF=	mapedit.xcf toolbar.xcf

MAN3=	mapedit.3 mapview.3
CATMAN3=mapedit.cat3 mapview.cat3
PSMAN3=	mapedit.ps3 mapview.ps3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
