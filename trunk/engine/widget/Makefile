#	$Csoft: Makefile,v 1.19 2002/06/14 23:13:39 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	fonts

LIB=	widget
SRCS=	widget.c window.c region.c text.c label.c button.c checkbox.c \
	textbox.c keycodes.c primitive.c mapview.c
OBJS=	widget.o window.o region.o text.o label.o button.o checkbox.o \
	textbox.o keycodes.o primitive.o mapview.o
FOBJS=	widget.fob window.fob

CFLAGS+=	${SDL_CFLAGS}

all:	widget.fob widget_offs.h window.fob window_offs.h

widget.fob widget_offs.h: widget.xcf
	${FOBJCOMP} -o widget.fob ${XCF}

window.fob window_offs.h: window.xcf
	${FOBJCOMP} -o window.fob ${XCF}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
