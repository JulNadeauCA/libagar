#	$Csoft: Makefile,v 1.4 2002/04/20 12:31:26 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	widget
SRCS=	widget.c window.c text.c label.c button.c
OBJS=	widget.o window.o text.o label.o button.o
SHARE=	LICENSE.ttf \
	dignity.ttf kingrich.ttf larabie.ttf pakenham.ttf suigener.ttf \
	zekton.ttf zerothre.ttf zerotwos.ttf zrnic.ttf
FOBJS=	widget.fob
XCF=	button.xcf

CFLAGS+=	${SDL_CFLAGS}

all:	widget.fob widget_offs.h

widget.fob widget_offs.h: ${XCF}
	${FOBJCOMP} -o widget.fob ${XCF}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
