# Agar-GUI Library
# Cross-platform GUI toolkit

# Base source files (always compiled when GUI is enabled)
gui_sources = files(
  'box.c',
  'button.c',
  'checkbox.c',
  'colors.c',
  'combo.c',
  'console.c',
  'controller.c',
  'cursors.c',
  'debugger.c',
  'dev_browser.c',
  'dev_classinfo.c',
  'dev_config.c',
  'dev_fonts.c',
  'dev_object_edit.c',
  'dev_timer_inspector.c',
  'dev_unicode_browser.c',
  'dir_dlg.c',
  'drv.c',
  'drv_dummy.c',
  'drv_mw.c',
  'drv_sw.c',
  'editable.c',
  'file_dlg.c',
  'fixed.c',
  'fixed_plotter.c',
  'font_selector.c',
  'font.c',
  'font_bf.c',
  'geometry.c',
  'global_keys.c',
  'glview.c',
  'graph.c',
  'gui.c',
  'hsvpal.c',
  'icon.c',
  'iconmgr.c',
  'input_device.c',
  'joystick.c',
  'keyboard.c',
  'keymap.c',
  'keymap_compose.c',
  'keymap_latin1.c',
  'keysyms.c',
  'label.c',
  'load_bmp.c',
  'load_color.c',
  'load_surface.c',
  'menu.c',
  'menu_view.c',
  'mfspinbutton.c',
  'mouse.c',
  'mpane.c',
  'mspinbutton.c',
  'notebook.c',
  'numerical.c',
  'objsel.c',
  'packedpixel.c',
  'pane.c',
  'pixmap.c',
  'primitive.c',
  'progress_bar.c',
  'radio.c',
  'scrollbar.c',
  'scrollview.c',
  'separator.c',
  'slider.c',
  'socket.c',
  'statusbar.c',
  'style_editor.c',
  'stylesheet.c',
  'surface.c',
  'table.c',
  'text.c',
  'text_cache.c',
  'textbox.c',
  'time_sdl.c',
  'titlebar.c',
  'tlist.c',
  'toolbar.c',
  'treetbl.c',
  'ucombo.c',
  'units.c',
  'widget.c',
  'window.c',
)

# Conditional sources based on enabled features

# FreeType font engine
if get_option('freetype') and freetype_dep.found()
  gui_sources += files('font_ft.c')
endif

# SDL 1.2 support
if get_option('sdl') and sdl_dep.found()
  gui_sources += files(
    'drv_sdl_common.c',
    'drv_sdlfb.c',
  )
  if have_opengl
    gui_sources += files('drv_sdlgl.c')
  endif
endif

# SDL 2.0 support
if get_option('sdl2') and sdl2_dep.found()
  gui_sources += files(
    'drv_sdl2_common.c',
    'drv_sdl_keysyms.c',
    'drv_sdl2fb.c',
  )
  if have_opengl
    gui_sources += files(
      'drv_sdl2gl.c',
      'drv_sdl2mw.c',
    )
  endif
endif

# OpenGL support
if have_opengl
  gui_sources += files('drv_gl_common.c')

  # GLX driver (X11 + OpenGL)
  if have_glx
    gui_sources += files('drv_glx.c')
  endif

  # WGL driver (Windows + OpenGL)
  if host_system == 'windows'
    gui_sources += files('drv_wgl.c')
  endif
endif

# Cocoa driver (macOS)
if host_system == 'darwin'
  objc = meson.get_compiler('objc')
  if objc.has_header('Cocoa/Cocoa.h')
    gui_sources += files('drv_cocoa.m')
  endif
endif

# JPEG support
if jpeg_dep.found()
  gui_sources += files('load_jpg.c')
endif

# PNG support
if png_dep.found()
  gui_sources += files('load_png.c')
endif

# GUI library dependencies
gui_deps = [
  libag_core_dep,
  math_dep,
]

if get_option('freetype') and freetype_dep.found()
  gui_deps += freetype_dep
endif

if get_option('fontconfig') and fontconfig_dep.found()
  gui_deps += fontconfig_dep
endif

if get_option('sdl') and sdl_dep.found()
  gui_deps += sdl_dep
endif

if get_option('sdl2') and sdl2_dep.found()
  gui_deps += sdl2_dep
endif

if get_option('gl') and have_opengl
  gui_deps += gl_dep
endif

if get_option('x11') and have_x11
  gui_deps += x11_dep
endif

if get_option('xinerama') and xinerama_dep.found()
  gui_deps += xinerama_dep
endif

if jpeg_dep.found()
  gui_deps += jpeg_dep
endif

if png_dep.found()
  gui_deps += png_dep
endif

# Compile flags
gui_c_args = global_defs + warning_flags + [
  '-D_AGAR_GUI_INTERNAL',
]

if get_option('debug_surfaces')
  gui_c_args += '-DDEBUG_SURFACES'
endif

# Include directories
# Include '.' and '../core' for local headers, and config_inc for generated config headers
gui_inc = [include_directories('.', '../core'), config_inc]

# Build library (respects parent project's default_library option)
libag_gui = library('ag_gui',
  gui_sources,
  c_args: gui_c_args,
  include_directories: gui_inc,
  dependencies: gui_deps,
  version: agar_soversion,
  soversion: agar_soversion_major,
  install: true,
)

# Declare dependency
libag_gui_dep = declare_dependency(
  link_with: libag_gui,
  include_directories: gui_inc,
  dependencies: gui_deps,
)

# Override dependency for wrap support (both 'agar' and 'agar-gui')
meson.override_dependency('agar', libag_gui_dep)
meson.override_dependency('agar-gui', libag_gui_dep)

# Install headers
install_headers(
  'begin.h',
  'box.h',
  'button.h',
  'checkbox.h',
  'close.h',
  'colors.h',
  'combo.h',
  'console.h',
  'controller.h',
  'cursors.h',
  'dir_dlg.h',
  'drv.h',
  'drv_cocoa_keymap.h',
  'drv_gl_common.h',
  'drv_glx_keymaps.h',
  'drv_mw.h',
  'drv_sdl2_common.h',
  'drv_sdl_common.h',
  'drv_sw.h',
  'editable.h',
  'expand_byte8.h',
  'file_dlg.h',
  'file_dlg_common.h',
  'fixed.h',
  'fixed_plotter.h',
  'font.h',
  'font_bf.h',
  'font_ft.h',
  'font_selector.h',
  'fonts.h',
  'fonts_data.h',
  'geometry.h',
  'glview.h',
  'graph.h',
  'gui.h',
  'gui_math.h',
  'gui_pub.h',
  'hsvpal.h',
  'icon.h',
  'iconmgr.h',
  'icons.h',
  'icons_data.h',
  'inline_colors.h',
  'inline_primitive.h',
  'inline_surface.h',
  'inline_widget.h',
  'input_device.h',
  'joystick.h',
  'keyboard.h',
  'keymap.h',
  'label.h',
  'load_surface.h',
  'menu.h',
  'mfspinbutton.h',
  'mouse.h',
  'mpane.h',
  'mspinbutton.h',
  'notebook.h',
  'nullability.h',
  'numerical.h',
  'objsel.h',
  'opengl.h',
  'packedpixel.h',
  'palettes.h',
  'pane.h',
  'pixmap.h',
  'primitive.h',
  'progress_bar.h',
  'radio.h',
  'scrollbar.h',
  'scrollview.h',
  'sdl.h',
  'sdl2.h',
  'separator.h',
  'slider.h',
  'socket.h',
  'statusbar.h',
  'style_data.h',
  'stylesheet.h',
  'surface.h',
  'table.h',
  'text.h',
  'text_cache.h',
  'textbox.h',
  'titlebar.h',
  'tlist.h',
  'toolbar.h',
  'treetbl.h',
  'ucombo.h',
  'unicode_ranges.h',
  'units.h',
  'widget.h',
  'window.h',
  subdir: 'agar/gui',
)
install_symlink(
  'gui.h',
  install_dir : 'include/agar',
  pointing_to: 'gui/gui_pub.h')

# Install data files (fonts and style sheet)
install_data(
  'fonts/agar-minimal.agbf',
  'fonts/agar-minimal-12.png',
  'fonts/agar-ideograms.agbf',
  'fonts/agar-ideograms-12.png',
  'fonts/agar-ideograms-14.png',
  'fonts/agar-ideograms-16.png',
  'fonts/algue.ttf',
  'fonts/algue-bold.ttf',
  'fonts/algue-italic.ttf',
  'fonts/algue-bold-italic.ttf',
  'fonts/charter-bold-italic.otf',
  'fonts/charter-bold.otf',
  'fonts/charter-italic.otf',
  'fonts/charter.otf',
  'fonts/fira-sans-bold-italic.otf',
  'fonts/fira-sans-bold.otf',
  'fonts/fira-sans-condensed-bold-italic.otf',
  'fonts/fira-sans-condensed-bold.otf',
  'fonts/fira-sans-condensed-italic.otf',
  'fonts/fira-sans-condensed-light-italic.otf',
  'fonts/fira-sans-condensed-light.otf',
  'fonts/fira-sans-condensed.otf',
  'fonts/fira-sans-italic.otf',
  'fonts/fira-sans-light-italic.otf',
  'fonts/fira-sans-light.otf',
  'fonts/fira-sans.otf',
  'fonts/fraktur.ttf',
  'fonts/league-gothic.otf',
  'fonts/league-gothic-italic.otf',
  'fonts/league-gothic-condensed.otf',
  'fonts/league-gothic-condensed-italic.otf',
  'fonts/league-spartan.otf',
  'fonts/league-spartan-black.otf',
  'fonts/league-spartan-bold.otf',
  'fonts/league-spartan-extrabold.otf',
  'fonts/league-spartan-extralight.otf',
  'fonts/league-spartan-light.otf',
  'fonts/league-spartan-semibold.otf',
  'fonts/monoalgue.ttf',
  'fonts/monoalgue-bold.ttf',
  'fonts/monoalgue-italic.ttf',
  'fonts/monoalgue-bold-italic.ttf',
  'fonts/unialgue.ttf',
  install_dir: get_option('datadir') / 'agar' / 'fonts',
)

install_data(
  'style.css',
  'license.txt',
  install_dir: get_option('datadir') / 'agar',
)

# Generate pkg-config file
pkg.generate(
  libag_gui,
  name: 'agar',
  description: 'Agar cross-platform GUI toolkit',
  version: agar_version,
  subdirs: 'agar',
  requires: ['agar-core'],
)

# Install manpages
gui_man3 = files(
  'AG_AlphaFn.3',
  'AG_Box.3',
  'AG_Button.3',
  'AG_Checkbox.3',
  'AG_Color.3',
  'AG_Combo.3',
  'AG_Console.3',
  'AG_Cursor.3',
  'AG_CustomEventLoop.3',
  'AG_DirDlg.3',
  'AG_Driver.3',
  'AG_DriverCocoa.3',
  'AG_DriverDUMMY.3',
  'AG_DriverGLX.3',
  'AG_DriverMw.3',
  'AG_DriverSDL2FB.3',
  'AG_DriverSDL2GL.3',
  'AG_DriverSDL2MW.3',
  'AG_DriverSDLFB.3',
  'AG_DriverSDLGL.3',
  'AG_DriverSw.3',
  'AG_DriverWGL.3',
  'AG_Dummy.3',
  'AG_Editable.3',
  'AG_FileDlg.3',
  'AG_Fixed.3',
  'AG_FixedPlotter.3',
  'AG_FontSelector.3',
  'AG_GL.3',
  'AG_GLView.3',
  'AG_GlobalKeys.3',
  'AG_Graph.3',
  'AG_GuiDebugger.3',
  'AG_HSVPal.3',
  'AG_Icon.3',
  'AG_InitGraphics.3',
  'AG_InitVideoSDL.3',
  'AG_Keyboard.3',
  'AG_KeyMod.3',
  'AG_KeySym.3',
  'AG_Label.3',
  'AG_MPane.3',
  'AG_Menu.3',
  'AG_MouseButton.3',
  'AG_Notebook.3',
  'AG_Numerical.3',
  'AG_Pane.3',
  'AG_Pixmap.3',
  'AG_ProgressBar.3',
  'AG_Radio.3',
  'AG_Rect.3',
  'AG_Scrollbar.3',
  'AG_Scrollview.3',
  'AG_Separator.3',
  'AG_SizeSpec.3',
  'AG_Slider.3',
  'AG_Socket.3',
  'AG_StyleSheet.3',
  'AG_Surface.3',
  'AG_Table.3',
  'AG_Text.3',
  'AG_Textbox.3',
  'AG_Titlebar.3',
  'AG_Tlist.3',
  'AG_Toolbar.3',
  'AG_Treetbl.3',
  'AG_UCombo.3',
  'AG_Units.3',
  'AG_Widget.3',
  'AG_WidgetPrimitives.3',
  'AG_Window.3',
)
install_man(gui_man3)
