#	$Csoft: Makefile,v 1.16 2002/05/26 00:51:08 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	fonts

LIB=	widget
SRCS=	widget.c window.c region.c text.c label.c button.c checkbox.c \
	textbox.c keycodes.c primitive.c
OBJS=	widget.o window.o region.o text.o label.o button.o checkbox.o \
	textbox.o keycodes.o primitive.o
FOBJS=	widget.fob
XCF=	widget.xcf

CFLAGS+=	${SDL_CFLAGS}

all:	widget.fob widget_offs.h

widget.fob widget_offs.h: ${XCF}
	${FOBJCOMP} -o widget.fob ${XCF}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
