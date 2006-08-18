#	$Csoft: Makefile,v 1.90 2005/10/01 09:50:55 vedge Exp $

TOP=	../
include ${TOP}/Makefile.config

SUBDIR=	fonts

LIB=		ag_gui
LIB_SHARED=	Yes
LIB_STATIC=	No

SHARESRC=	gui-pixmaps.den

SRCS=	widget.c window.c box.c text.c label.c button.c checkbox.c \
	textbox.c keycodes.c keysyms.c primitive.c radio.c graph.c \
	tlist.c scrollbar.c spinbutton.c titlebar.c toolbar.c \
	fspinbutton.c combo.c ucombo.c units.c nlunits.c hsvpal.c \
	mspinbutton.c mfspinbutton.c statusbar.c menu.c menu_view.c \
	tableview.c separator.c colors.c cursors.c notebook.c pixmap.c \
	animview.c file_dlg.c hpane.c vpane.c objsel.c fixed.c \
	table.c glview.c unicode.c ucslcpy.c ucslcat.c ucssep.c mpane.c pane.c \
	console.c
MAN3=	AG_Widget.3 AG_Button.3 AG_Graph.3 AG_Checkbox.3 AG_Label.3 AG_Radio.3 \
	AG_Textbox.3 AG_Window.3 AG_Scrollbar.3 AG_Tlist.3 AG_Text.3 AG_Box.3 \
	AG_HBox.3 AG_VBox.3 AG_Combo.3 AG_FSpinbutton.3 AG_UCombo.3 \
	AG_Spinbutton.3 AG_Units.3 AG_HSVPal.3 AG_Menu.3 AG_SizeSpec.3


CFLAGS+=${SDL_CFLAGS} ${FREETYPE_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
