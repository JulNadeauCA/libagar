#	$Csoft: Makefile,v 1.29 2002/08/13 02:32:11 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool

LIB=	mapedit
SRCS=	mapedit.c fileops.c toolbar.c mapview.c mapwin.c \
	tilestack.c objq.c
OBJS=	mapedit.o fileops.o toolbar.o mapview.o mapwin.o \
	tilestack.o objq.o
FOBJS=	mapedit.fob

CFLAGS+=	${SDL_CFLAGS}

XCF=	mapedit.xcf props.xcf toolbar.xcf

all:	mapedit.fob mapedit_offs.h

depend:	mapedit.fob

mapedit.fob mapedit_offs.h: ${XCF}
	${FOBJCOMP} -o mapedit.fob ${XCF}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
