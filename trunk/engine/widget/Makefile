#	$Csoft: Makefile,v 1.14 2002/05/25 08:50:13 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	fonts keymaps

LIB=	widget
SRCS=	widget.c window.c region.c text.c label.c button.c checkbox.c \
	textbox.c keycodes.c
OBJS=	widget.o window.o region.o text.o label.o button.o checkbox.o \
	textbox.o keycodes.o
FOBJS=	widget.fob
XCF=	widget.xcf

CFLAGS+=	${SDL_CFLAGS}

all:	widget.fob widget_offs.h

widget.fob widget_offs.h: ${XCF}
	${FOBJCOMP} -o widget.fob ${XCF}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
