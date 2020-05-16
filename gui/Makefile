TOP=..

include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

LIB=		ag_gui
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_INSTALL=	Yes
LIB_GUID=	"71a13512-13e7-44a6-ba69-e868296cdc34"
LIB_LINKS=	${CORE_LINKS_STATIC} ${GUI_LIBS}
DATAFILES_SRC=	style.css \
		license.txt
SUBDIR=		fonts

MAN3=	AG_Widget.3 AG_Button.3 AG_FixedPlotter.3 AG_Checkbox.3 \
	AG_Label.3 AG_Radio.3 AG_Textbox.3 AG_Window.3 AG_Scrollbar.3 \
	AG_Tlist.3 AG_Text.3 AG_Box.3 AG_Combo.3 AG_UCombo.3 AG_Units.3 \
	AG_HSVPal.3 AG_Menu.3 AG_SizeSpec.3 AG_Table.3 AG_Pane.3 AG_MPane.3 \
	AG_Fixed.3 AG_Pixmap.3 AG_Notebook.3 AG_Toolbar.3 AG_FileDlg.3 \
	AG_Console.3 AG_GLView.3 AG_Graph.3 AG_Icon.3 AG_Socket.3 AG_Cursor.3 \
	AG_Separator.3 AG_Numerical.3 AG_ProgressBar.3 \
	AG_Editable.3 AG_Slider.3 AG_Surface.3 AG_Scrollview.3 AG_Rect.3 \
	AG_Titlebar.3 AG_FontSelector.3 AG_Treetbl.3 AG_GuiDebugger.3 \
	AG_WidgetPrimitives.3 AG_CustomEventLoop.3 AG_AlphaFn.3 AG_KeySym.3 \
	AG_KeyMod.3 AG_Color.3 AG_MouseButton.3 AG_Driver.3 AG_InitGraphics.3 \
	AG_DriverMw.3 AG_DriverSw.3 AG_DriverGLX.3 AG_DriverWGL.3 \
	AG_DriverSDLFB.3 AG_DriverSDLGL.3 AG_DriverDUMMY.3 AG_GL.3 AG_DirDlg.3 \
	AG_GlobalKeys.3 AG_Keyboard.3 AG_DriverCocoa.3 AG_InitVideoSDL.3 \
	AG_StyleSheet.3

SRCS=	${SRCS_GUI} drv.c drv_sw.c drv_mw.c drv_dummy.c primitive.c \
	gui.c widget.c window.c iconmgr.c geometry.c \
	colors.c cursors.c ttf.c text.c keymap.c keymap_latin1.c \
	keymap_compose.c keysyms.c editable.c \
	box.c label.c button.c checkbox.c textbox.c radio.c \
	fixed_plotter.c tlist.c scrollbar.c titlebar.c toolbar.c \
	combo.c ucombo.c units.c nlunits.c hsvpal.c \
	mspinbutton.c mfspinbutton.c statusbar.c menu.c menu_view.c \
	treetbl.c separator.c notebook.c pixmap.c file_dlg.c objsel.c \
	fixed.c numerical.c table.c glview.c mpane.c pane.c console.c graph.c \
	socket.c icon.c progress_bar.c slider.c text_cache.c load_surface.c \
	load_color.c load_xcf.c scrollview.c font_selector.c \
	time_sdl.c debugger.c surface.c global_keys.c \
	input_device.c mouse.c keyboard.c packedpixel.c load_bmp.c load_jpg.c \
	load_png.c dir_dlg.c stylesheet.c style_editor.c \
	dev_browser.c dev_classinfo.c dev_config.c dev_cpuinfo.c dev_fonts.c \
	dev_object_edit.c dev_timer_inspector.c dev_unicode_browser.c

CFLAGS+=${CORE_CFLAGS} \
	${GUI_CFLAGS} -D_AGAR_GUI_INTERNAL

LIBS=	-L../core -lag_core \
	${GUI_LIBS} \
	${CORE_LIBS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk

fonts_data.h: ${SRCDIR}/gui/fonts/algue.ttf
	bundlefont -o $@ -v -n agFontAlgue ${SRCDIR}/gui/fonts/algue.ttf
	bundlefont -o $@ -a -v -n agFontAlgue_Bold ${SRCDIR}/gui/fonts/algue-bold.ttf
	bundlefont -o $@ -a -v -n agFontAlgue_Italic ${SRCDIR}/gui/fonts/algue-italic.ttf
	bundlefont -o $@ -a -v -n agFontAlgue_BoldItalic ${SRCDIR}/gui/fonts/algue-bold-italic.ttf
#	bundlefont -o $@ -a -b -n agFontMinimal ${SRCDIR}/gui/fonts/minimal.xcf
#	bundlefont -o $@ -a -b -n agFontMinimalSerif ${SRCDIR}/gui/fonts/serif.xcf

style_data.h: ${SRCDIR}/gui/style.css
	bundlecss -o $@ -n agStyleDefault ${SRCDIR}/gui/style.css

style:
	rm -f style_data.h
	env SRCDIR=.. ${MAKE} style_data.h

fonts:
	rm -f fonts_data.h
	env SRCDIR=.. ${MAKE} fonts_data.h

.PHONY: style fonts
