#	$Csoft: Makefile,v 1.1.1.1 2002/01/25 09:50:02 vedge Exp $

TOP=	../..

FOBJS=	mapedit.fob

all:	mapedit.fob mapedit_offs.h

mapedit.fob mapedit_offs.h: mapedit.xcf
	${FOBJCOMP} -o mapedit.fob mapedit.xcf

include ${TOP}/Makefile.inc
