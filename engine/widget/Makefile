#	$Csoft: Makefile,v 1.47 2003/06/10 06:44:54 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	fonts

LIB=	widget
SRCS=	widget.c window.c box.c text.c label.c button.c checkbox.c \
	textbox.c keycodes.c primitive.c radio.c graph.c bitmap.c tlist.c \
	scrollbar.c palette.c titlebar.c spinbutton.c combo.c
OBJS=	widget.o window.o box.o text.o label.o button.o checkbox.o \
	textbox.o keycodes.o primitive.o radio.o graph.o bitmap.o tlist.o \
	scrollbar.o palette.o titlebar.o spinbutton.o combo.o

FOBJ=	window.fob
FOBDIR=	/engine/widget
FOFFS=	window_offs.h
XCF=	window.xcf

MAN3=	widget.3 button.3 graph.3 \
	checkbox.3 label.3 radio.3 \
	textbox.3 window.3 bitmap.3 \
	scrollbar.3 tlist.3 palette.3 \
	text.3 box.3 hbox.3 \
	vbox.3
CATMAN3=widget.cat3 button.cat3 graph.cat3 \
	checkbox.cat3 label.cat3 radio.cat3 \
	textbox.cat3 window.cat3 bitmap.cat3 \
	scrollbar.cat3 tlist.cat3 palette.cat3 \
	text.cat3 box.cat3 hbox.cat3 \
	vbox.cat3
PSMAN3=	widget.ps3 button.ps3 graph.ps3 \
	checkbox.ps3 label.ps3 radio.ps3 \
	textbox.ps3 window.ps3 bitmap.ps3 \
	scrollbar.ps3 tlist.ps3 palette.ps3 \
	text.ps3 box.ps3 hbox.ps3 \
	vbox.ps3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/libfobj/Makefile.inc
include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
