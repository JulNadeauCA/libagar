TOP=..
include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc

SUBDIR=	fonts

LIB=		ag_gui
LIB_MAJOR=	4
LIB_MINOR=	0
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"71a13512-13e7-44a6-ba69-e868296cdc34"
LIB_LINKS=	${CORE_LINKS_STATIC} SDL opengl freetype

SRCS=	gui.c widget.c window.c style.c style_default.c iconmgr.c \
	colors.c cursors.c ttf.c text.c keymap_ascii.c keymap_utf8.c \
	keymap_latin1.c keymap_compose.c keysyms.c editable.c \
	box.c label.c button.c checkbox.c textbox.c radio.c \
	fixed_plotter.c tlist.c scrollbar.c spinbutton.c titlebar.c toolbar.c \
	fspinbutton.c combo.c ucombo.c units.c nlunits.c hsvpal.c \
	mspinbutton.c mfspinbutton.c statusbar.c menu.c menu_view.c \
	treetbl.c separator.c notebook.c pixmap.c file_dlg.c objsel.c \
	fixed.c numerical.c table.c glview.c mpane.c pane.c console.c graph.c \
	socket.c icon.c progress_bar.c slider.c text_cache.c load_surface.c \
	load_color.c load_xcf.c file_selector.c scrollview.c font_selector.c \
	time_sdl.c debugger.c surface.c perfmon.c widget_legacy.c global_keys.c \
	input_device.c mouse.c keyboard.c packedpixel.c load_bmp.c load_jpg.c \
	load_png.c drv.c drv_sw.c drv_mw.c drv_gl_common.c drv_sdl_common.c \
	drv_glx.c drv_sdlfb.c drv_sdlgl.c drv_wgl.c dir_dlg.c anim.c

MAN3=	AG_Widget.3 AG_Button.3 AG_FixedPlotter.3 AG_Checkbox.3 \
	AG_Label.3 AG_Radio.3 AG_Textbox.3 AG_Window.3 AG_Scrollbar.3 \
	AG_Tlist.3 AG_Text.3 AG_Box.3 AG_HBox.3 AG_VBox.3 AG_Combo.3 \
	AG_FSpinbutton.3 AG_UCombo.3 AG_Spinbutton.3 AG_Units.3 AG_HSVPal.3 \
	AG_Menu.3 AG_SizeSpec.3 AG_Table.3 AG_Pane.3 AG_MPane.3 AG_Fixed.3 \
	AG_Pixmap.3 AG_Colors.3 AG_Notebook.3 AG_Toolbar.3 AG_FileDlg.3 \
	AG_Console.3 AG_GLView.3 AG_Graph.3 AG_Icon.3 AG_Socket.3 AG_Cursor.3 \
	AG_Separator.3 AG_Numerical.3 AG_Style.3 AG_ProgressBar.3 \
	AG_Editable.3 AG_Slider.3 AG_Surface.3 AG_Scrollview.3 AG_Rect.3 \
	AG_Titlebar.3 AG_FontSelector.3 AG_Treetbl.3 AG_GuiDebugger.3 \
	AG_WidgetPrimitives.3 AG_EventLoop.3 AG_BlendFn.3 AG_KeySym.3 \
	AG_KeyMod.3 AG_Color.3 AG_MouseButton.3 AG_Driver.3 AG_InitGraphics.3 \
	AG_DriverMw.3 AG_DriverSw.3 AG_DriverGLX.3 AG_DriverWGL.3 \
	AG_DriverSDLFB.3 AG_DriverSDLGL.3 AG_GL.3 AG_DirDlg.3

CFLAGS+=-D_AGAR_GUI_INTERNAL ${SDL_CFLAGS} ${FREETYPE_CFLAGS} ${OPENGL_CFLAGS} \
	${X11_CFLAGS} ${JPEG_CFLAGS} ${PNG_CFLAGS} ${MATH_CFLAGS} \
	${GETTEXT_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
