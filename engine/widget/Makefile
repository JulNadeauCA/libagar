#	$Csoft: Makefile,v 1.80 2005/03/24 04:02:07 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	fonts \
	pixmaps

LIB=		widget
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	widget.c window.c box.c text.c label.c button.c checkbox.c \
	textbox.c keycodes.c keysyms.c primitive.c radio.c graph.c bitmap.c \
	tlist.c scrollbar.c palette.c spinbutton.c titlebar.c toolbar.c \
	fspinbutton.c combo.c ucombo.c units.c nlunits.c mgraph.c hsvpal.c \
	mspinbutton.c mfspinbutton.c statusbar.c menu.c menu_view.c \
	tableview.c separator.c colors.c cursors.c notebook.c \
	animview.c file_dlg.c
MAN3=	widget.3 button.3 graph.3 checkbox.3 label.3 radio.3 textbox.3 \
	window.3 bitmap.3 scrollbar.3 tlist.3 palette.3 text.3 box.3 \
	hbox.3 vbox.3 combo.3 fspinbutton.3 ucombo.3 spinbutton.3 units.3

CFLAGS+=${SDL_CFLAGS} ${FREETYPE_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
