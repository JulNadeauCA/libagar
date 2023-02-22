# TODO

## Developer Tools

- Make [**Style Editor**](https://libagar.org/man3/AG_StyleEditor) and [**GUI Debugger**](https://libagar.org/man3/AG_GuiDebugger) more robust against different kinds of VFS surgery.
- The Agar Interface Builder [**agarib**](https://libagar.org/man1/agarib).
- The Agar Debugger [**agardb**](https://libagar.org/man1/agardb).
- Build system: Add [**CMake**](https://cmake.org) support.

## Bindings

- Ada: Add Widget classes to the new Ada bindings in `ada/gui/`.
- Perl: Split compilation into multiple units instead of one giant `Agar.xs` (can it be done?)

## Packaging and Distribution

- Update FreeBSD `devel/agar`.
- Update OpenBSD `devel/agar`.
- Submit ports/packages for `agartest` and tools such as `mapedit`, `rgedit`, `sgedit`, `skedit` and `vgedit`.
- Submit ports/packages for `p5-Agar` and `agar-ada`.
- Suggest ports for [GnuCOBOL](https://sourceforge.net/projects/open-cobol/) and [GuiCOBOL](http://www.opencobol.altervista.org).

## Style and Typography Engine

- [**AG_StyleSheet**](https://libagar.org/man3/AG_StyleSheet): Add an "E > F" style selector "Class > #id" for named instances of attached child widgets.
- [**AG_StyleSheet**](https://libagar.org/man3/AG_StyleSheet): Gradients, textured BG and border styles.
- Integrate the HarfBuzz text shaping engine for better handling of complex script.

## Widgets

- [**AG_Console**](https://libagar.org/man3/AG_Console): Handle ANSI cursor and terminal operations. Clipboard integration.
- [**AG_FontSelector**](https://libagar.org/man3/AG_FontSelector): Allow the user to enter arbitrary sample text in the Preview field.
- [**AG_Notebook**](https://libagar.org/man3/AG_Notebook): Fix padding issues. Add disposition modes Bottom, Left & Right. Improve keyboard/controller navigation.
- [**AG_Pixmap**](https://libagar.org/man3/AG_Pixmap) & [**AG_Fixed**](https://libagar.org/man3/AG_Fixed): Zoom operations.
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Filters. Stencil operations.
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Implement loading of animated surfaces from GIF files.
- [**AG_Textbox**](https://libagar.org/man3/AG_Textbox) & [**AG_Editable**](https://libagar.org/man3/AG_Editable): Extend SGR support. Syntax highlighting & rich-text editing methods.
- [**AG_WidgetPrimitives**](https://libagar.org/man3/AG_WidgetPrimitives): Dithering. Shadow effects.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Provide a variation of the "zoom" feature to allow the user to zoom individual widgets.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Threading optimizations. Cache the `VISIBLE` flag into an `int` so that culling / rendering and mouse-intersection tests can skip over invisible widgets without locking them.
- [**AG_Window**](https://libagar.org/man3/AG_Window): MRU API to simplify the process of remembering geometries. New gravity methods for autoplacing.

## Drivers / Ports

- [**cocoa**](https://libagar.org/man3/AG_DriverCocoa): Cursor operations. Clipboard integration.
- [**glx**](https://libagar.org/man3/AG_DriverGLX): Fix `AG_Icon` drag-and-drop (e.g., `sockets` test). Possibly using `AG_WindowMove()`?
- [**glx**](https://libagar.org/man3/AG_DriverGLX), [**wgl**](https://libagar.org/man3/AG_DriverWGL): Pump / regulate events to make live window resize more responsive. Right now no effort is made to update the display quickly when they are resized. Can we do this without hurting performance?
- [**sdl2mw**](https://libagar.org/man3/AG_DriverSDL2MW): Cursor operations. Clipboard integration.
- [**wgl**](https://libagar.org/man3/AG_DriverWGL): Implement clipboard integration.
- An accelerated driver for GameCube GX (devKitPro).
- A software framebuffer driver for X Windows. 

