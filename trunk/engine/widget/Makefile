#	$Csoft: Makefile,v 1.36 2002/12/24 09:44:04 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	fonts

LIB=	widget
SRCS=	widget.c window.c region.c text.c label.c button.c checkbox.c \
	textbox.c keycodes.c primitive.c radio.c graph.c bitmap.c tlist.c \
	scrollbar.c palette.c
OBJS=	widget.o window.o region.o text.o label.o button.o checkbox.o \
	textbox.o keycodes.o primitive.o radio.o graph.o bitmap.o tlist.o \
	scrollbar.o palette.o

MAN3=	widget.3 button.3 graph.3 checkbox.3 label.3 radio.3 \
	region.3 textbox.3 window.3 bitmap.3 scrollbar.3 \
	tlist.3
CATMAN3=widget.cat3 button.cat3 graph.cat3 checkbox.cat3 label.cat3 radio.cat3 \
	region.cat3 textbox.cat3 window.cat3 bitmap.cat3 scrollbar.cat3 \
	tlist.cat3


CFLAGS+=	${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
