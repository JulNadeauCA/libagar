#	$Csoft: Makefile,v 1.56 2003/11/19 11:36:30 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	fonts

LIB=	widget
SRCS=	widget.c window.c box.c text.c label.c button.c checkbox.c \
	textbox.c keycodes.c primitive.c radio.c graph.c bitmap.c tlist.c \
	scrollbar.c palette.c titlebar.c spinbutton.c fspinbutton.c combo.c \
	ucombo.c units.c
MAN3=	widget.3 button.3 graph.3 checkbox.3 label.3 radio.3 textbox.3 \
	window.3 bitmap.3 scrollbar.3 tlist.3 palette.3 text.3 box.3 \
	hbox.3 vbox.3 combo.3 fspinbutton.3

XCF=		window.xcf
DEN=		window.den
DEN_DIR=	engine/widget
DEN_NAME=	"Window titlebar icons"
DEN_AUTHOR=	"vedge@"
DEN_COPYRIGHT=	"2003 CubeSoft Communications, Inc."

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/dencomp/Makefile.inc
include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
