#	$Csoft: Makefile,v 1.25 2002/07/27 08:59:06 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	fonts

SHARE=	README.graph

LIB=	widget
SRCS=	widget.c window.c region.c text.c label.c button.c checkbox.c \
	textbox.c keycodes.c primitive.c radio.c graph.c
OBJS=	widget.o window.o region.o text.o label.o button.o checkbox.o \
	textbox.o keycodes.o primitive.o radio.o graph.o
FOBJS=	widget.fob window.fob

MAN3=	widget.3
CATMAN3=widget.cat3

CFLAGS+=	${SDL_CFLAGS}

all:	widget.fob widget_offs.h window.fob window_offs.h

widget.fob widget_offs.h: widget.xcf
	${FOBJCOMP} -o widget.fob widget.xcf

window.fob window_offs.h: window.xcf
	${FOBJCOMP} -o window.fob window.xcf

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
