#	$Csoft: Makefile,v 1.29 2002/08/22 04:50:20 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	fonts

SHARE=	README.graph

LIB=	widget
SRCS=	widget.c window.c region.c text.c label.c button.c checkbox.c \
	textbox.c keycodes.c primitive.c radio.c graph.c bitmap.c
OBJS=	widget.o window.o region.o text.o label.o button.o checkbox.o \
	textbox.o keycodes.o primitive.o radio.o graph.o bitmap.o
FOBJS=	widget.fob window.fob

MAN3=	widget.3 button.3 graph.3 checkbox.3 label.3 radio.3 \
	region.3 textbox.3 window.3 bitmap.3
CATMAN3=widget.cat3 button.cat3 graph.cat3 checkbox.cat3 label.cat3 radio.cat3 \
	region.cat3 textbox.cat3 window.cat3 bitmap.cat3

CFLAGS+=	${SDL_CFLAGS}

all:	widget.fob widget_offs.h window.fob window_offs.h

widget.fob widget_offs.h: widget.xcf
	${FOBJCOMP} -o widget.fob widget.xcf

window.fob window_offs.h: window.xcf
	${FOBJCOMP} -o window.fob window.xcf

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
include ${TOP}/libfobj/Makefile.inc
