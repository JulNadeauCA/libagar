#	$Csoft: Makefile,v 1.28 2002/07/29 05:29:52 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool

LIB=	mapedit
SRCS=	mapedit.c fileops.c toolbar.c mapview.c mapwin.c \
	tilestack.c tileq.c objq.c
OBJS=	mapedit.o fileops.o toolbar.o mapview.o mapwin.o \
	tilestack.o tileq.o objq.o
FOBJS=	mapedit.fob

CFLAGS+=	${SDL_CFLAGS}

XCF=	mapedit.xcf props.xcf toolbar.xcf

all:	mapedit.fob mapedit_offs.h

depend:	mapedit.fob

mapedit.fob mapedit_offs.h: ${XCF}
	${FOBJCOMP} -o mapedit.fob ${XCF}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
