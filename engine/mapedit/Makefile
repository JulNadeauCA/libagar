#	$Csoft: Makefile,v 1.35 2003/01/27 08:00:00 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool

LIB=	mapedit
SRCS=	mapedit.c fileops.c mapview.c objq.c
OBJS=	mapedit.o fileops.o mapview.o objq.o

FOBJS=	mapedit.fob mapview.fob
FOFFS=	mapedit_offs.h mapview_offs.h

MAN3=	mapedit.3 mapview.3
CATMAN3=mapedit.cat3 mapview.cat3

CFLAGS+=${SDL_CFLAGS}

XCF0=	mapedit.xcf toolbar.xcf
XCF1=	props.xcf

all:	${FOBJS} ${FOFFS}

depend:	mapedit.fob mapview.fob

mapedit.fob mapedit_offs.h: ${XCF0}
	${FOBJCOMP} -o mapedit.fob ${XCF0}

mapview.fob mapview_offs.h: ${XCF1}
	${FOBJCOMP} -o mapview.fob ${XCF1}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
