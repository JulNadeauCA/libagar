#	$Csoft$

TOP=	../..

FOBJS=	mapedit.fob

all:	mapedit.fob mapedit_offs.h

mapedit.fob mapedit_offs.h: mapedit.xcf
	${FOBJCOMP} -o ${.TARGET} mapedit.xcf

include ${TOP}/Makefile.inc
