#	$Csoft: Makefile,v 1.30 2002/08/18 00:34:24 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool

LIB=	mapedit
SRCS=	mapedit.c fileops.c toolbar.c mapview.c mapwin.c \
	tilestack.c objq.c
OBJS=	mapedit.o fileops.o toolbar.o mapview.o mapwin.o \
	tilestack.o objq.o
FOBJS=	mapedit.fob

MAN3=	mapedit.3 mapview.3 objq.3 tilestack.3
CATMAN3=mapedit.cat3 mapview.cat3 objq.cat3 tilestack.cat3

CFLAGS+=	${SDL_CFLAGS}

XCF=	mapedit.xcf props.xcf toolbar.xcf

all:	mapedit.fob mapedit_offs.h

depend:	mapedit.fob

mapedit.fob mapedit_offs.h: ${XCF}
	${FOBJCOMP} -o mapedit.fob ${XCF}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
