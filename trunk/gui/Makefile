TOP=	../
include ${TOP}/Makefile.config

SUBDIR=	fonts

LIB=		ag_gui
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"71a13512-13e7-44a6-ba69-e868296cdc34"

LIB_LINKS=	ag_core SDL

#SHARESRC=	gui-pixmaps.den

SRCS=	widget.c window.c box.c text.c label.c button.c checkbox.c \
	textbox.c keycodes.c keysyms.c primitive.c radio.c fixed_plotter.c \
	tlist.c scrollbar.c spinbutton.c titlebar.c toolbar.c \
	fspinbutton.c combo.c ucombo.c units.c nlunits.c hsvpal.c \
	mspinbutton.c mfspinbutton.c statusbar.c menu.c menu_view.c \
	tableview.c separator.c colors.c cursors.c notebook.c pixmap.c \
	file_dlg.c objsel.c fixed.c numerical.c \
	table.c glview.c unicode.c ucslcpy.c ucslcat.c ucssep.c mpane.c pane.c \
	console.c graph.c ttf.c scrollable.c scrollable_text.c socket.c icon.c
MAN3=	AG_Widget.3 AG_Button.3 AG_FixedPlotter.3 AG_Checkbox.3 AG_Label.3 \
	AG_Radio.3 AG_Textbox.3 AG_Window.3 AG_Scrollbar.3 AG_Tlist.3 \
	AG_Text.3 AG_Box.3 AG_HBox.3 AG_VBox.3 AG_Combo.3 AG_FSpinbutton.3 \
	AG_UCombo.3 AG_Spinbutton.3 AG_Units.3 AG_HSVPal.3 AG_Menu.3 \
	AG_SizeSpec.3 AG_Table.3 AG_Pane.3 AG_MPane.3 AG_Fixed.3 AG_Pixmap.3 \
	AG_Colors.3 AG_Notebook.3 AG_Toolbar.3 AG_FileDlg.3 AG_Console.3

CFLAGS+=-I.. ${SDL_CFLAGS} ${FREETYPE_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
