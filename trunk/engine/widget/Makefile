#	$Csoft: Makefile,v 1.18 2002/06/14 21:22:34 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	fonts

LIB=	widget
SRCS=	widget.c window.c region.c text.c label.c button.c checkbox.c \
	textbox.c keycodes.c primitive.c mapview.c
OBJS=	widget.o window.o region.o text.o label.o button.o checkbox.o \
	textbox.o keycodes.o primitive.o mapview.o
FOBJS=	widget.fob window.fob
XCF=	widget.xcf window.xcf

CFLAGS+=	${SDL_CFLAGS}

all:	widget.fob widget_offs.h window.fob window_offs.h

widget.fob widget_offs.h: ${XCF}
	${FOBJCOMP} -o widget.fob ${XCF}

window.fob window_offs.h: ${XCF}
	${FOBJCOMP} -o window.fob ${XCF}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
