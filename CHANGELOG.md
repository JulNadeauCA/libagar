# Changelog
All notable changes to Agar will be documented in this file. The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
- Build system: [**CMake**](https://cmake.org) support.
- A driver for the [**SDL 2.0**](https://libsdl.org) series.
- A software framebuffer driver for X Windows.
- Remote debugger [**agardb**](https://libagar.org/man1/agardb).
- Interface builder to help the process of constructing static or fixed-resolution interfaces.
- Add widget classes to the new Ada bindings in `ada/gui/`.
- [**AG_Console**](https://libagar.org/man3/AG_Console): Handle ansi cursor and terminal operations. Clipboard integration.
- [**AG_FontSelector**](https://libagar.org/man3/AG_FontSelector): Display only those styles that are available for a given font (as opposed to a static list of styles).
- [**AG_Menu**](https://libagar.org/man3/AG_Menu): Implement focusability and keyboard navigation methods. Handle "font-changed" better under multi-window drivers.
- [**AG_Notebook**](https://libagar.org/man3/AG_Notebook): Add disposition modes Bottom, Left & Right.
- [**AG_Pixmap**](https://libagar.org/man3/AG_Pixmap) & [**AG_Fixed**](https://libagar.org/man3/AG_Fixed): Zoom operations.
- [**AG_StyleEditor**](https://libagar.org/man3/AG_StyleEditor): Edition of stylesheet rules.
- [**AG_StyleSheet**](https://libagar.org/man3/AG_StyleSheet): Gradients and border styles. New selector `"E > F"` (an `F` element child of an `E` element).
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Filters. Stencil operations.
- [**AG_Textbox**](https://libagar.org/man3/AG_Textbox) & [**AG_Editable**](https://libagar.org/man3/AG_Editable): Extend SGR support. Syntax highlighting & rich-text editing methods.
- [**AG_WidgetPrimitives**](https://libagar.org/man3/AG_WidgetPrimitives): Dithering. Shadow effects.
- [**AG_Window**](https://libagar.org/man3/AG_Window): MRU API to simplify the process of remembering geometries. New gravity methods for autoplacing.
- [**cocoa**](https://libagar.org/man2/AG_DriverCocoa): Cursor operations. Clipboard integration.
- [**glx**](https://libagar.org/man3/AG_DriverGLX): Make drag-and-drop (`sockets` test) work correctly (possibly using `AG_WindowMove()`).
- [**wgl**](https://libagar.org/man2/AG_DriverWGL): Clipboard integration. Live resize. Fix cursor bugs.

## [1.6.0] - 2020-05-16
### Added
- Integrated [**Style Editor**](https://libagar.org/man3/AG_StyleEditor) tool. It allows a developer to inspect a live VFS of widgets, to pick elements, to add/edit style attributes and look at the results in real time.
- Integrated [**GUI Debugger**](https://libagar.org/man3/AG_GuiDebugger) utility. Inspect a live VFS of widgets in real time. Available in Debug builds only.
- [**AG_Button**](https://libagar.org/man3/AG_Button): New functions [AG_ButtonGetState()](https://libagar.org/man3/AG_ButtonGetState), [AG_ButtonSetState](https://libagar.org/man3/AG_ButtonSetState) and atomic [AG_ButtonToggle()](https://libagar.org/man3/AG_ButtonToggle). New option flag `SET` to force initial _"state"_ to **1**). New option flag `RETURN_BUTTON` to embed a "Return" button which raises _"textbox-return"_ when pressed.
- [**AG_Checkbox**](https://libagar.org/man3/AG_Checkbox): Display a check mark (U+2713). Thanks Federico!
- [**AG_Checkbox**](https://libagar.org/man3/AG_Checkbox): New option flag `INVERT`.
- [**AG_Config**](https://libagar.org/man3/AG_Config): New settings: "Tab Width", "Cursor Blink Rate", "Mouse Scroll Interval", "Enable `GL_DEBUG_OUTPUT`" and "NPOT (non power of two) textures".
- [**AG_Console**](https://libagar.org/man3/AG_Console): Handle multiline entries. [AG_ConsoleMsg()](https://libagar.org/man3/AG_ConsoleMsg) will now split multiline strings into multiple, grouped lines that are displayed in an indented style. Thanks Chuck!
- [**AG_Console**](https://libagar.org/man3/AG_Console): Introduce [event sink](https://libagar.org/man3/AG_EventSink)-based file monitoring features. New functions [AG_ConsoleOpenFile](https://libagar.org/man3/AG_ConsoleOpenFile), [AG_ConsoleOpenFD](https://libagar.org/man3/AG_ConsoleOpenFD), [AG_ConsoleOpenStream()](https://libagar.org/man3/AG_ConsoleOpenStream) and [AG_ConsoleClose()](https://libagar.org/man3/AG_ConsoleClose).
- [**AG_Console**](https://libagar.org/man3/AG_Console): New function [AG_ConsoleBinary()](https://libagar.org/man3/AG_ConsoleBinary) to produce data in canonical (hex + ASCII) format. New function [AG_ConsoleMsgCatS()](https://libagar.org/man3/AG_ConsoleMsgCatS) for appending to an existing entry.
- [**AG_Console**](https://libagar.org/man3/AG_Console): New function [AG_ConsoleExportBuffer()](https://libagar.org/man3/AG_ConsoleExportBuffer) for exporting entire buffer contents. Added "Selected Lines Only" option in "Export to file". Added menu function "Clear All". Added image types under "Export to file" for exporting screenshots.
- [**AG_DataSource**](https://libagar.org/man3/AG_DataSource): New fixed-length string encoding functions [AG_CopyStringPadded()](https://libagar.org/man3/AG_CopyStringPadded), [AG_WriteStringPadded()](https://libagar.org/man3/AG_WriteStringPadded) and [AG_SkipStringPadded()](https://libagar.org/man3/AG_SkipStringPadded).
- [**AG_Driver**](https://libagar.org/man3/AG_Driver): New operations: `putPixel64()`, `putPixelRGB16()`, `drawTriangle()`, `drawPolygon()`, `drawPolygonSti32()`, `drawLineW`(), `drawLineW_Sti16()`, `getClipboardText()` and `setClipboardText()`.
- [**AG_Editable**](https://libagar.org/man3/AG_Editable) & [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): Implement basic support for ANSI SGR attributes.
- [**AG_Editable**](https://libagar.org/man3/AG_Editable) & [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): Added autocomplete feature. New functions [AG_EditableAutocomplete()](https://libagar.org/man3/AG_EditableAutocomplete) and [AG_EditableCloseAutocomplete()](https://libagar.org/man3/AG_EditableCloseAutocomplete).
- [**AG_Editable**](https://libagar.org/man3/AG_Editable) & [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): New property _"placeholder"_ to specify a text label to show whenever the buffer is empty. Thanks Federico for the suggestion!
- [**AG_Editable**](https://libagar.org/man3/AG_Editable) & [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): New options `UPPERCASE` and `LOWERCASE`.
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): Added [AG_FileDlgAddImageTypes()](https://libagar.org/man3/AG_FileDlgAddImageTypes).
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): New `COMPACT` option. In this mode, the widget becomes a single-line ( Textbox & Button ). The Button triggers the FileDlg to instantiate a clone of itself in a new window. New constructors [AG_FileDlgNewCompact()](https://libagar.org/man3/AG_FileDlgNewCompact) and [AG_FileDlgNewCompactMRU()](https://libagar.org/man3/AG_FileDlgNewCompactMRU).
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): Added special syntaxes to `AG_FileType` item. `"<-x>"` tests whether a file is executable by the effective user. `"<=FILENAME>"` matches filenames exactly. `"<=FILENAME/i>"` performs case-insensitive matching.
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): New functions [AG_FileDlgGetDirectory()](https://libagar.org/man3/AG_FileDlgGetDirectory), [AG_FileDlgGetFilename()](https://libagar.org/man3/AG_FileDlgGetFilename), [AG_FileDlgCopyDirectory()](https://libagar.org/man3/AG_FileDlgCopyDirectory), [AG_FileDlgCopyFilename()](https://libagar.org/man3/AG_FileDlgCopyFilename) and [AG_FileDlgCopyTypes()](https://libagar.org/man3/AG_FileDlgCopyTypes).
- [**AG_Graph**](https://libagar.org/man3/AG_Graph): Add support for directed graphs. Edges now include distinct types `UNDIRECTED` and `DIRECTED`. Thanks Chuck!
- [**AG_HSVPal**](https://libagar.org/man3/AG_HSVPal): New functions [AG_HSVPalUpdateHue()](https://libagar.org/man3/AG_HSVPalUpdateHue) and [AG_HSVPalUpdateSV()](https://libagar.org/man3/AG_HSVPalUpdateSV).
- [**AG_Notebook**](https://libagar.org/man3/AG_Notebook): New functions [AG_NotebookGetByID()](https://libagar.org/man3/AG_NotebookGetByID) and [AG_NotebookSelectByID()](https://libagar.org/man3/AG_NotebookSelectByID) for referencing tabs by integer ID.
- [**AG_Object**](https://libagar.org/man3/AG_Object): New run-time object validity and class-membership tests. In Debug builds, passing an invalid object pointer to any Agar API call should now trigger "Illegal access" assertions. Additional tests are done when traversing linked lists in order to detect memory errors.
- [**AG_Object**](https://libagar.org/man3/AG_Object): New functions [AG_CreateClass()](https://libagar.org/man3/AG_CreateClass) and [AG_DestroyClass()](https://libagar.org/man3/AG_DestroyClass). Provide an auto-allocated alternative to [AG_RegisterClass()](https://libagar.org/man3/AG_RegisterClass) interface (which takes a pre-initialized argument). Added [AG_ClassSetInit()](https://libagar.org/man3/AG_ClassSetInit), [AG_ClassSetReset()](https://libagar.org/man3/AG_ClassSetReset), [AG_ClassSetDestroy()](https://libagar.org/man3/AG_ClassSetDestroy), [AG_ClassSetLoad()](https://libagar.org/man3/AG_ClassSetLoad), [AG_ClassSetSave()](https://libagar.org/man3/AG_ClassSetSave) and [AG_ClassSetEdit()](https://libagar.org/man3/AG_ClassSetEdit) for setting and overriding function pointers dynamically.
- [**AG_Object**](https://libagar.org/man3/AG_Object): New property _"archive-path"_. Deprecate `AG_ObjectGetArchivePath()` and `AG_ObjectSetArchivePath()`.
- [**AG_Pixmap**](https://libagar.org/man3/AG_Pixmap): New function [AG_PixmapGetSurface()](https://libagar.org/man3/AG_PixmapGetSurface) to return a copy of surface at a given index.
- [**AG_Radio**](https://libagar.org/man3/AG_Radio): Handle multiline items. Implement key repeat.
- [**AG_Radio**](https://libagar.org/man3/AG_Radio): Implement horizontal layout. New function [AG_RadioSetDisposition()](https://libagar.org/man3/AG_RadioSetDisposition). Thanks Federico!
- [**AG_Separator**](https://libagar.org/man3/AG_Separator): New function [AG_SeparatorSetLength()](https://libagar.org/man3/AG_SeparatorSetLength) to set a requisition when placing separators in containers of indeterminate size.
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Add support for 48- and 64-bit surfaces (under `LARGE` memory model).
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Add support for 1-bit (monochrome), 2-bit (4-color), and 4-bit (16 color) palettized modes. Cache `PixelsPerByte` in `format` field.
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Add support for Grayscale+Alpha modes (32-bit and 64-bit).
- [**AG_Text**](https://libagar.org/man3/AG_Text): Add support for ANSI SGR attributes in [AG_TextSize()](https://libagar.org/man3/AG_TextSize) and [AG_TextRender()](https://libagar.org/man3/AG_TextRender). Map the SGR Alternative Fonts to our core fonts.
- [**AG_Text**](https://libagar.org/man3/AG_Text): New functions [AG_TextFontPctFlags()](https://libagar.org/man3/AG_TextFontPctFlags) and [AG_CopyTextState()](https://libagar.org/man3/AG_CopyTextState).
- [**AG_Timer**](https://libagar.org/man3/AG_Timer): New functions [AG_ExecTimer()](https://libagar.org/man3/AG_ExecTimer) & [AG_DelTimers()](https://libagar.org/man3/AG_DelTimers).
- [**AG_Tlist**](https://libagar.org/man3/AG_Tlist): Implement per-item alternate colors and font flags. New functions [AG_TlistSetColor()](https://libagar.org/man3/AG_TlistSetColor) and [AG_TlistSetFont()](https://libagar.org/man3/AG_TlistSetFont) to style items individually.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New function [AG_SetStyleF()](https://libagar.org/man3/AG_SetStyleF).
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New events _"font-changed"_ and _"palette-changed"_. They are generated by the style compiler to signal whenever a widget's `font` or any entries in its palette (`pal`) have been changed.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New functions [AG_PushBlendingMode()](https://libagar.org/man3/AG_PushBlendingMode) & [AG_PopBlendingMode()](https://libagar.org/man3/AG_PopBlendingMode).
- [**AG_Window**](https://libagar.org/man3/AG_Window): New function [AG_WindowMove()](https://libagar.org/man3/AG_WindowMove) for moving windows more efficiently when no resize is required.
- New color manipulation routines [AG_ColorDarken()](https://libagar.org/man3/AG_ColorDarken), [AG_ColorLighten()](https://libagar.org/man3/AG_ColorLighten) and [AG_ColorInterpolate()](https://libagar.org/man3/AG_ColorInterpolate).
- New size hinting functions [AG_PixmapSizeHint()](https://libagar.org/man3/AG_PixmapSizeHint), [AG_BoxSizeHint()](https://libagar.org/man3/AG_BoxSizeHint), [AG_RadioSizeHint()](https://libagar.org/man3/AG_RadioSizeHint) and [AG_FixedSizeHint()](https://libagar.org/man3/AG_FixedSizeHint), for specifying explicit size requisitions in pixels.
- New functions [AG_ClipLine()](https://libagar.org/man3/AG_ClipLine) for clipping lines to rectangular bounding boxes, [AG_ClipLineCircle()](https://libagar.org/man3/AG_ClipLineCircle) for clipping lines to circular regions. New function [AG_GetLineIntersection()](https://libagar.org/man3/AG_GetLineIntersection) for computing the intersection of two line segments. Thanks Chuck!
- New function [AG_DrawArrowhead()](https://libagar.org/man3/AG_DrawArrowhead) for drawing arrowheads of arbitrary size and angle aligned to arbitrary vectors. New function [AG_DrawArrowLine()](https://libagar.org/man3/AG_DrawArrowLine) for drawing lines with arrowheads. Thanks Chuck!
- New function [AG_About()](https://libagar.org/man3/AG_About) to generate a simple "About Agar" dialog with license text.
- [**dummy**](https://libagar.org/man3/AG_DriverDUMMY): A new **no-op** driver which prints out calls and arguments to the console. In [agartest](https://libagar.org/man1/agartest), use the "-C" option to see the output (e.g., "agartest -C -d dummy").
- [**glx**](https://libagar.org/man3/AG_DriverGLX): New driver option _"xsync"_ to enable synchronous X events (e.g., `"-d glx(xsync)"`). This is useful when debugging issues involving any call into the X Window system.
- New [style](https://libagar.org/man3/AG_StyleSheet) attributes for colors. Primaries are _"color"_, _"background-color"_, _"text-color"_, _"line-color"_, _"high-color"_, _"low-color"_ and _"selection-color"_. States are _#unfocused_, _#disabled_, _#focused_ and _#hover_.
- New [style](https://libagar.org/man3/AG_StyleSheet) attributes _"font-family"_, _"font-size"_, _"font-weight"_, _"font-style"_ and _"font-stretch"_.
- New [style](https://libagar.org/man3/AG_StyleSheet) attribute _"padding"_. Allowed syntaxes are `"padding: X"` (set all paddings) and `"padding: T R B L"` (set Top, Right, Bottom & Left separately). For example, the padding of an [AG_Button](https://libagar.org/man3/AG_Button) sets the space in pixels between the text label and the button's outer edges. The padding of an [AG_Box](https://libagar.org/man3/AG_Box) sets the padding around the entire set of attached child widgets.
- New [style](https://libagar.org/man3/AG_StyleSheet) attribute _"spacing"_. Allowed syntaxes are `"spacing: X"` (set both spacings) and `"spacing: H V"` (set horizontal and vertical spacings separately).  In a vertical [AG_Radio](https://libagar.org/man3/AG_Radio) for example, `H` sets the horizontal space between the radio button and its label and `V` sets the vertical space in pixels between radio items.  The spacing of an [AG_Box](https://libagar.org/man3/AG_Box) sets the spacing between its child widgets.
- New fonts. The distribution now includes a set of [OFL 1.1](https://scripts.sil.org/OFL) licensed ***core fonts*** in order to help improve cross-platform typography:
	- #0: _Algue_ (not a RFN. Latin & graphical icons. Baked into _ag_gui_).
	- #1: _Unialgue_ (not a RFN. Latin Extended, Arabic, Armenian, Canadian Aboriginal, Cyrillic, Devanagari, Georgian, Greek Extended, Gujarati, Gurmukhi, Hebrew, Lao, Lisu, Nko, Ogham, Tamil, Thai, Tibetan & Tifinagh).
	- #2: An unused slot.
	- #3: [_Computer Modern Unicode Sans_](https://cm-unicode.sourceforge.io) (Latin Extended, Greek & Cyrillic).
	- #4: _Computer Modern Unicode Serif_ (Latin Extended, Greek & Cyrillic).
	- #5: _Computer Modern Unicode Typewriter_ (Latin Extended, Greek & Cyrillic).
	- #6: [_Bitstream Charter_](https://en.wikipedia.org/wiki/Bitstream_Charter) (Latin).
	- #7: [_Courier Prime_](https://quoteunquoteapps.com/courierprime) (Latin).
	- #8: [_Source Han Sans_](https://github.com/adobe-fonts/source-han-sans) (Latin, Chinese, Japanese & Korean).
	- #9: [_League Spartan_](https://www.theleagueofmoveabletype.com/league-spartan) (Latin).
	- #10: [_League Gothic_](https://www.theleagueofmoveabletype.com/league-gothic) (Latin).
	- #11: [_Unifraktur Maguntia_](http://unifraktur.sourceforge.net/maguntia.html) (Latin).
- Clipboard integration (currently implemented in [**glx**](https://libagar.org/man3/AG_DriverGLX)). Added new [AG_Driver](https://libagar.org/man3/AG_Driver) operations `getClipboardText()` and `setClipboardText()`.
- Provide `dlsym()`mable copies of inline functions in lowercase form. For example, the symbol `AG_LengthUTF8()` is now guaranteed to have a dlsymmable copy called `ag_length_utf8()`. This helps avoid the need for glue code in language bindings. Thanks Brian and Federico!
- **Nullability**: Introduce `_Nullable` and `_Nonnull` pointer annotations to help prevent programming mistakes and make prototypes more expressive.
- Define special nullability annotations for thread types which require special handling since they may or may not be pointer types depending on the platform. Define `_{Nullable,Nonnull,Null_unspecified}_{Mutex,Cond,Thread}`.
- New pointer-type-safe accessor macros for event handlers. Instead of using the generic pointer accessors `AG_SELF()`, `AG_PTR()` or `AG_PTR_NAMED()` to retrieve an object pointer from an event handler, one can now use the more specific `AG_OBJECT_SELF()`, `AG_OBJECT_PTR()` and `AG_OBJECT_PTR_NAMED()` macros. In a _Debug_ build, such macros will perform a run-time validity and class-membership test. In a _Release_ build, no tests are done. Class-specific accessor macros are also provided, for example [AG_Button](https://libagar.org/man3/AG_Button) defines `AG_BUTTON_SELF()`, `AG_BUTTON_PTR()` and `AG_BUTTON_PTR_NAMED()`.
- New build option `--enable-type-safety` (implied by `--enable-debug`).
- New build option `--with-memory-model`. `SMALL` targets 8-bit machines (for _ag_micro_ only). `MEDIUM` has a smaller memory footprint and can handle up to 24-bit "True Color" surfaces (the default on non 64-bit hosts). The `LARGE` model adds support for 48-bit "Deep Color" surfaces (the default on 64-bit hosts). Either `MEDIUM` or `LARGE` will work on both 32-bit and 64-bit hosts (the choice comes down to performance tuning). Introduce `AG_Size` and `AG_Offset` types.
- New build option `--disable-widgets` build with base framework, but without the standard widget library.
- _ag_core_: Import [rxi](https://github.com/rxi)'s type-safe dynamic arrays [vec](https://github.com/rxi/vec). Thanks Chuck for the suggestion!
- _ag_core_: Colorize debugging output. Added `--disable-ansi-color` build option.
- _ag_core_: New [AG_SetErrorS()](https://libagar.org/man3/AG_SetErrorS) function variant.
- _ag_math_: New functions [AG_Square()](https://libagar.org/man3/AG_Square), [AG_HaveQuadraticSolution()](https://libagar.org/man3/AG_HaveQuadraticSolution), [AG_QuadraticPositive()](https://libagar.org/man3/AG_QuadraticPositive), [AG_QuadraticNegative()](https://libagar.org/man3/AG_QuadraticNegative) & [AG_Distance()](https://libagar.org/man3/AG_Distance). Thanks Chuck!
- _ag_math_: New [**M_Bezier**](https://libagar.org/man3/M_Bezier) module for computing Bézier curves. Thanks Chuck!
- _ag_net_: Introduce [**AG_Web**](https://libagar.org/man3/AG_Web), a multiprocess HTTP/1.1 application server. It handles authentication, session/process management, push events and templates (not in default build, must use `--enable-web`).
- _ag_vg_: Make [**VG**](https://libagar.org/man3/VG) an [AG_Object](https://libagar.org/man3/AG_Object) class since we may wish to subclass it or set variables and events on it.
- _ag_vg_: Introduce [**vgedit**](https://libagar.org/man1/vgedit), a basic editor for [**VG**](https://libagar.org/man3/VG) object files. It is also a good demonstration of [**VG_View**](https://libagar.org/man3/VG_View) widget usage.

### Removed
- [**AG_Event**](https://libagar.org/man3/AG_Event): In [AG_PostEvent()](https://libagar.org/man3/AG_PostEvent), remove the mandatory "sender" first argument. Sender objects can be passed instead as regular arguments.
- [**AG_Event**](https://libagar.org/man3/AG_Event): Removed the `ASYNC` and `PROPAGATE` option flags. Both behaviors can be implemented more flexibly in the event handler routine itself.
- [**AG_Event**](https://libagar.org/man3/AG_Event): Removed the `child-attached` and `child-detached` events.
- [**AG_Object**](https://libagar.org/man3/AG_Object): Removed the `AG_ObjectDep` structure and linkage. Dependencies are now represented with less overhead using [AG_Variables](https://libagar.org/man3/AG_Variable).
- [**AG_Object**](https://libagar.org/man3/AG_Object): Removed the `save_pfx` and `archivePath` fields (replaced by the _"archive-path"_ property).
- [**AG_Object**](https://libagar.org/man3/AG_Object): Remove typed virtual functions. This improves performance and allows `AG_Get*()` functions to be declared `pure` in unthreaded builds.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Removed the options `NOSPACING` and `TABLE_EMBEDDABLE` which are no longer relevant.

### Changed
- [**AG_Button**](https://libagar.org/man3/AG_Button): Renamed `AG_ButtonInvertState()` -> [AG_ButtonSetInverted()](https://libagar.org/man3/AG_ButtonSetInverted).
- [**AG_Button**](https://libagar.org/man3/AG_Button), [**AG_Checkbox**](https://libagar.org/man3/AG_Checkbox) & [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): Implement rendering of text labels internally to avoid the cost of embedding [AG_Labels](https://libagar.org/man3/AG_Label) widgets.
- [**AG_Color**](https://libagar.org/man3/AG_Color): Handle web keywords (`"AliceBlue"`, `"antiquewhite"`, etc). They can now be used in widget color attributes and stylesheets. Keywords are case-insensitive.
- [**AG_Color**](https://libagar.org/man3/AG_Color): Handle shortened `"#RGB"` and `"#RGBA"` formats in [AG_ColorFromString()](https://libagar.org/man3/AG_ColorFromString).
- [**AG_Combo**](https://libagar.org/man3/AG_Combo) & [**AG_UCombo**](https://libagar.org/man3/AG_UCombo): Cache generated windows to improve response time under [multi-window drivers](https://libagar.org/man3/AG_DriverMw).
- [**AG_Console**](https://libagar.org/man3/AG_Console): Rendering optimizations. Avoid redundant blending in draw() by pre-caching lines against an opaque background.
- [**AG_Config**](https://libagar.org/man3/AG_Config): New functions [AG_ConfigFind()](https://libagar.org/man3/AG_ConfigFind), [AG_ConfigAddPath()](https://libagar.org/man3/AG_ConfigAddPath), [AG_ConfigDelPath()](https://libagar.org/man3/AG_ConfigDelPath) and [AG_ConfigSetPath()](https://libagar.org/man3/AG_ConfigSetPath). Replaces former colon-separated _"load-path"_, _"save-path"_, _"font-path"_, and _"tmp-path"_ attributes. Define standard path groups `PATH_DATA`, `PATH_FONTS` and `PATH_TEMP`.
- [**AG_Editable**](https://libagar.org/man3/AG_Editable) & [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): Improve keyboard handling. Use 4 independent keyrepeat timers for the direction keys.
- [**AG_Editable**](https://libagar.org/man3/AG_Editable) & [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): Improvements to the word-selection behavior (triggered by ALT or double-click followed by mouse motion).
- [**AG_FontSelector**](https://libagar.org/man3/AG_FontSelector): Include Agar core fonts and user fonts in the listing (scanning _"font-path"_ for files with supported extensions). Add _"pixel64"_ binding in Large mode. Embed a color picker to preview fonts in different colors. Add Upright Italic and width variants. Add an alternate sample phrase.
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): Refresh automatically when directory contents change when running on platforms with support for filesystem notifications (using an [event sink](https://libagar.org/man3/AG_EventSink) of type `FSEVENT`).
- [**AG_Menu**](https://libagar.org/man3/AG_Menu): Improve responsiveness of menu expansions. Draw a rectangular outline to avoid blending into the background.
- [**AG_Notebook**](https://libagar.org/man3/AG_Notebook): Legibility and rendering fixes. Render a stripe in _"selected-color"_ over the selected tab. Handle multiline text correctly in tab labels.
- [**AG_Object**](https://libagar.org/man3/AG_Object): Make [AG_Variables](https://libagar.org/man3/AG_Variable) of type `P_FLAG*` mutable to `INT` and `UINT*` types. In [AG_CompareVariables()](https://libagar.org/man3/AG_CompareVariables), compare discrete `STRING` variables by string value as opposed to comparing by reference. The `AG_GetVariableLocked()` function was renamed [AG_AccessVariable()](https://libagar.org/man3/AG_AccessVariable). Dependencies are now represented more compactly as AG\_Variables of type `P_OBJECT` and `P_VARIABLE`.
- [**AG_Scrollbar**](https://libagar.org/man3/AG_Scrollbar): Enhance contrast when control bar is squished below a constant threshold. Adjust the size requisition based on the zoom level (as opposed to the font size). Added `SMOOTH` option to produce a progressive motion towards the target for clicks outside of the control bar (as opposed to jumping which is the default).
- [**AG_StyleSheet**](https://libagar.org/man3/AG_StyleSheet): The stylesheet parser now handles C-style comments. Improved validation and error messages.
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Pack `AG_PixelFormat` in the `AG_Surface` structure itself (so `format` is no longer a pointer).
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Set the `MAPPED` bit on surfaces that have been mapped with [AG_WidgetMapSurface()](https://libagar.org/man3/AG_WidgetMapSurface). Detect attempts at freeing surfaces without first unmapping them.
- [**AG_Text**](https://libagar.org/man3/AG_Text): Inner-loop optimizations in [AG_TextSize()](https://libagar.org/man3/AG_TextSize) and [AG_TextRender()](https://libagar.org/man3/AG_TextRender) backends. Reorganized code to use jump tables and allow new types of font engines to be used. Safety improvements in the backend of [AG_{Push,Pop}TextState()](https://libagar.org/man3/AG_PushTextState).
- [**AG_Text**](https://libagar.org/man3/AG_Text): Handle fractional (floating-point) font sizes. This affords the style engine greater precision whenever _"font-size"_ is expressed in "%".
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Store GL state data in a separate auto-allocated structure `AG_WidgetGL` to reduce overhead in the non-`USE_OPENGL` case.
- [**AG_WidgetPrimitives**](https://libagar.org/man3/AG_WidgetPrimitives): Produce smoother transitions between edges and corners of 3D-style primitives such as [AG_DrawBox()](https://libagar.org/man3/AG_DrawBox) and [AG_DrawBoxRounded()](https://libagar.org/man3/AG_DrawBoxRounded).
- Pass [**AG_Color**](https://libagar.org/man3/AG_Color) and [**AG_Rect**](https://libagar.org/man3/AG_Rect) by reference in public APIs.
- Use the `const`, `pure`, `noreturn`, `malloc_like` and `warn_unused_result` attributes when they are available. Introduce `_Pure_Attribute_If_Unthreaded`.
- Optimize the case where a window contains no widgets that make use of text rendering and therefore require no push/popping of the text state. Cache the result under `AG_WINDOW_USE_TEXT` in parent window.
- Improvements, fixes and optimizations in the [**sizing routines**](https://libagar.org/man3/AG_Widget#SIZING) of widgets in general.
- Moved [**AG_Net**](https://libagar.org/man3/AG_Net) code from _ag_core_ to a separate library called ***ag_net***. This prevents network-call-related security warnings on some platforms and packaging systems from affecting programs which do not use network calls.
- [**glx**](https://libagar.org/man3/AG_DriverGLX): Obtain initial keyboard repeat delay and interval settings from [XKB](https://www.x.org/wiki/XKB). Fallback to legacy XF86MISC if XKB is not available.
- [**sdlfb**](https://libagar.org/man3/AG_DriverSDLFB) & [**sdlgl**](https://libagar.org/man3/AG_DriverSDLGL): Handle _"width"_ and _"height"_ parameters in % (relative to available desktop space). Added _"!bgPopup"_ option to disable background popup menu. Handle "R/G/B" format in _"bgColor"_ parameter.
- [**sdlfb**](https://libagar.org/man3/AG_DriverSDLFB) & [**sdlgl**](https://libagar.org/man3/AG_DriverSDLGL): Allow windows (including minimized window icons) to move past view boundaries.
- [Build system](https://bsdbuild.hypertriton.com) updates. Fix endianness test for mingw and darwin hosts. It is no longer necessary to pass `--byte-order` when cross-compiling to mingw. The build system now handles Windows DLL files as regular binaries. We no longer use libtool. Added auto-generation of pkg-config .pc modules.
- New build options `--without-inline` to disable inlining entirely and `--without-inline-*` to disable selectively.
- Manual page improvements (clarity, wording, examples).
- New Ada bindings (in ada/). Updated bindings are variable-thickness and offer the ability to implement new Agar object classes in pure Ada.
- Mat's Perl bindings (in p5-Agar/) have been updated. Fixed bootstrapping code to work with recent perls.
- _ag_core_: The "posix" module of [**AG_User**](https://libagar.org/man3/AG_User) uses `getpwent()` which may incur some startup overhead for applications under some platforms. Introduce a new module **getenv**, which avoids the call and obtains the information instead from the `$USER`, `$[E]UID`, `$HOME` and `$TMPDIR` environment variables. It is now the default unless `AG_POSIX_USERS` is passed to [AG_InitCore()](https://libagar.org/man3/AG_InitCore).
- _ag_core_: Compile cleanly on small C compilers such as [cc65](https://www.cc65.org) and [sdcc](https://sdcc.sourceforge.net).
- _ag_core_: Make [AG_GetVariable()](https://libagar.org/man3/AG_GetVariable) a non-variadic function. This may cause some new compiler warnings which can be avoided by casting the argument to `(void *)`.

### Fixed
- [**AG_Console**](https://libagar.org/man3/AG_Console): Fixed memory leaks. Handle empty lines more efficiently.
- [**AG_DSO**](https://libagar.org/man3/AG_DSO): In OSX Lion (10.7) and later, prefer dlopen() over dyld() method.
- [**AG_HSVPal**](https://libagar.org/man3/AG_HSVPal): Fixed rendering issues. Scale the different features better under different sizes. Avoid overdraw in the transparency preview. If displaying an RGB or HSV text label, select a sensible text color based on HSV parameters. Short-circuit the cases where s or v are close to 0 (according to `AG_SATURATION_EPSILON` and `AG_VALUE_EPSILON`).
- [**AG_Menu**](https://libagar.org/man3/AG_Menu): Fixed collapse behavior when closing hierarchies of opened menus. Fixed incorrect alignment of submenus in some circumstances. Fix modal behavior of menu expansions in [multi-window drivers](https://libagar.org/man3/AG_DriverMw).
- [**AG_Label**](https://libagar.org/man3/AG_Label): In `POLLED` mode, obtain better size requisitions by processing the format string earlier in `size_request()`. This prevents the need for manual size hinting in common cases.
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Fix the PNG loader and exporter. PNG files with 16-bit per channel are now supported. Handle indexed-mode images correctly.
- [**AG_Text**](https://libagar.org/man3/AG_Text): Fix a rendering problem with certain fonts where glyphs would blend incorrectly with the previously rendered glyphs.
- [**AG_Tlist**](https://libagar.org/man3/AG_Tlist): Make [AG_TlistSort()](https://libagar.org/man3/AG_TlistSort) thread-safe.
- [**AG_Tlist**](https://libagar.org/man3/AG_Tlist): When right-clicking, select the target item before triggering popup menus.
- [**glx**](https://libagar.org/man3/AG_DriverGLX): If the initial `glXChooseVisual()` attempt fails, fallback to an 8-bit depth.
- [**glx**](https://libagar.org/man3/AG_DriverGLX): Use XGrabPointer() to handle windows with `MODAL` flag. Thanks Chuck!
- Fixed some pixel off-by-one errors in [widget primitives](https://libagar.org/man3/AG_WidgetPrimitives).
- Fixed an unwanted side effect when passing an argument of `NULL` or `agWindowFocused` to [AG_WindowFocus()](https://libagar.org/man3/AG_WindowFocus). It would cause any previous focus request to be cancelled. It is now a proper no-op.
- Thread-safety and efficiency improvements. Removed many unnecessary lock operations.
- Fixed 32-bit MSYS build (include missing header file). Thanks Varialus!
- _ag_core_: It is now safe for [Event Sink](https://libagar.org/man3/AG_AddEventSink) routines to call [AG_DelEventSink()](https://libagar.org/man3/AD_DelEventSink) on themselves.
- _ag_core_: It is now safe for an [Event Epilogue](https://libagar.org/man3/AG_AddEventEpilogue) routine to call [AG_DelEventEpilogue()](https://libagar.org/man3/AD_DelEventEpilogue) on itself. Thanks Walter Zambotti!

## [1.5.0] - 2016-03-25
### Added
- [**AG_Button**](https://libagar.org/man3/AG_Button): New option `EXCL` to optimize for exclusive access to binding.
- [**AG_Color**](https://libagar.org/man3/AG_Color): New structure to represent RGBA color internally. New function [AG_ColorFromString()](https://libagar.org/man3/AG_ColorFromString) to convert from a string representation.
- [**AG_Color**](https://libagar.org/man3/AG_Color): New native color structure.
- [**AG_Console**](https://libagar.org/man3/AG_Console): New operations "Export", "Select all" & "Copy". New function [AG_ConsoleExportText()](https://libagar.org/man3/AG_ConsoleExportText).
- [**AG_DataSource**](https://libagar.org/man3/AG_DataSource): New functions [AG_OpenNetSocket()](https://libagar.org/man3/AG_OpenNetSocket) and [AG_DataSourceRealloc()](https://libagar.org/man3/AG_DataSourceRealloc). Make the API extensible so new data source types can be defined.
- [**AG_Db**](https://libagar.org/man3/AG_Db): New interface to access databases of key-value pairs. Included backends are "hash", "btree" and "mysql".
- [**AG_DirDlg**](https://libagar.org/man3/AG_DirDlg): New widget for selecting directories. It provides an alternative to [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg) which limits the selection to directories.
- [**AG_Driver**](https://libagar.org/man3/AG_Driver): New functions [AG_UsingGL](https://libagar.org/man3/AG_UsingGL) and [AG_UsingSDL](https://libagar.org/man3/AG_UsingSDL).
- [**AG_Editable**](https://libagar.org/man3/AG_Editable): Implement scrolling queries. Allow seeking to specified pixel coordinates, lines or character index.
- [**AG_Editable**](https://libagar.org/man3/AG_Editable): New [Buffer Access Routines](https://libagar.org/man3/AG_Editable#BUFFER_ACCESS_ROUTINES). Provides a more advanced API for accessing and editing buffer contents programmatically.
- [**AG_Editable**](https://libagar.org/man3/AG_Editable) & [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): Implement binding to multilingual [**AG_TextElement**](https://libagar.org/man3/AG_TextElement). New `MULTILINGUAL` option. New function [AG_TextboxSetLang](https://libagar.org/man3/AG_TextboxSetLang).
- [**AG_Editable**](https://libagar.org/man3/AG_Editable) & [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): New clipboard feature. New option `EXCL` to optimize based on the widget having exclusive access over the string.
- [**AG_File**](https://libagar.org/man3/AG_File) (in _ag_core_): New function [AG_RegisterFileExtMappings(3)](https://libagar.org/man3/AG_RegisterFileExtMappings).
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): New options `MASK_HIDDEN` ("Mask hidden files") and `MASK_EXT` ("Filter by extension").
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): Use the [**AG_User**](https://libagar.org/man3/AG_User) API to get the home directory path.
- [**AG_GLView**](https://libagar.org/man3/AG_GLView): New option `BGFILL`. New underlay function.
- [**AG_HSVPal**](https://libagar.org/man3/AG_HSVPal): New binding _"color"_ for editing colors in native [**AG_Color**](https://libagar.org/man3/AG_Color) format.
- [**AG_Menu**](https://libagar.org/man3/AG_Menu): New function [AG_MenuCollapseAll()](https://libagar.org/man3/AG_MenuCollapseAll).
- [**AG_Net**](https://libagar.org/man3/AG_Net): New API for cross-platform network access.
- [**AG_Object**](https://libagar.org/man3/AG_Object): New functions [AG_ObjectGetName()](https://libagar.org/man3/AG_ObjectGetName) which is an auto-allocated variant to [AG_ObjectCopyName()](https://libagar.org/man3/AG_ObjectCopyName).
- [**AG_Object**](https://libagar.org/man3/AG_Object): New function [AG_GetStringP()](https://libagar.org/man3/AG_GetStringP), an unsafe (non-duplicating) variant of [AG_GetString()](https://libagar.org/man3/AG_GetString).
- [**AG_Object**](https://libagar.org/man3/AG_Object): New functions [AG_ObjectGetInheritHier()](https://libagar.org/man3/AG_ObjectGetInheritHier) to obtain a list of classes an object inherits from. New function [AG_ObjectGetInheritHierString()](https://libagar.org/man3/AG_ObjectGetInheritHierString).
- [**AG_Printf**](https://libagar.org/man3/AG_Printf): New formatting engine which extends printf(3) functionality with new modifiers and support for registering formatting routines.
- [**AG_Printf**](https://libagar.org/man3/AG_Printf): New functions [AG_PrintfP()](https://libagar.org/man3/AG_PrintfP) and [AG_ProcessFmtString()](https://libagar.org/man3/AG_ProcessFmtString) for performing deferred output conversion. It is now used to implement [polled labels](https://libagar.org/man3/AG_LabelNewPolled) but is also usable in non-graphical applications.
- [**AG_ProgressBar**](https://libagar.org/man3/AG_ProgressBar): New option `EXCL` to optimize for exclusive access to binding.
- [**AG_Scrollbar**](https://libagar.org/man3/AG_Scrollbar): New options `EXCL` and `AUTOSIZE`. New _"inc"_ binding for setting the scrolling increment.
- [**AG_Slider**](https://libagar.org/man3/AG_Slider): New option `EXCL` to optimize assuming exclusive access to the binding data.
- [**AG_String**](https://libagar.org/man3/AG_String): New manual page for string-related routines.
- [**AG_StyleSheet**](https://libagar.org/man3/AG_StyleSheet): Implement basic style sheet parser. Color and typography attributes (such as _"color"_ or _"font-size"_) can be set on a per-widget basis by calling [AG_SetStyle()](https://libagar.org/man3/AG_SetStyle). Style attributes can also be set on a per-class basis from a stylesheet. A default stylesheet is provided in `gui/style.css`. Attributes are inherited from the parent by default.
- [**AG_Timer**](https://libagar.org/man3/AG_Timer): New timer API. The former `AG_Timeout()` interface is deprecated and now emulated. New timers are integrated with the [event loop](https://libagar.org/man3/AG_EventLoop) such that timers can work cross-platform and also provide thread-safety.
- [**AG_Text**](https://libagar.org/man3/AG_Text): New function [AG_TextTabWidth()](https://libagar.org/man3/AG_TextTabWidth) for setting rendering tab width (subject to [AG_PushTextState()](https://libagar.org/man3/AG_PushTextState) attribute stack). New function [AG_UnusedFont()](https://libagar.org/man3/AG_UnusedFont) for decrementing reference count on a font.
- [**AG_Text**](https://libagar.org/man3/AG_Text): Introduce [fontconfig](https://www.fontconfig.org) support in [AG_FetchFont()](https://libagar.org/man3/AG_FetchFont) wherever fontconfig is available.
- [**AG_TextElement**](https://libagar.org/man3/AG_TextElement): New interface for auto-allocated multi-lingual text buffers.
- [**AG_Tlist**](https://libagar.org/man3/AG_Tlist): New functions [AG_TlistSetIconWidth()](https://libagar.org/man3/AG_TlistSetIconWidth). New event type _"tlist-return"_.
- [**AG_Keyboard**](https://libagar.org/man3/AG_Keyboard): New functions [AG_LookupKeyName()](https://libagar.org/man3/AG_LookupKeyName), [AG_LookupKeySym()](https://libagar.org/man3/AG_LookupKeySym). Provide [AG_CompareKeyMods()](https://libagar.org/man3/AG_CompareKeyMods) for comparing keyboard modifier states.
- [**AG_Units**](https://libagar.org/man3/AG_Units): New conversion units: EnergyPerSubstanceAmount, MolarHeatCapacity, Resistivity, ThermalConductivity, ThermalExpansion & Density.
- [**AG_User**](https://libagar.org/man3/AG_User): New API to access user information such as username and path to home directory.
- [**AG_Variable**](https://libagar.org/man3/AG_Variable): New functions [AG_BindVariable()](https://libagar.org/man3/AG_BindVariable), [AG_DerefVariable()](https://libagar.org/man3/AG_DerefVariable) & [AG_CompareVariables()](https://libagar.org/man3/AG_CompareVariables).
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New `USE_MOUSEOVER` option and `MOUSEOVER` flag.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New `USE_TEXT` option. Introduce "font-changed" event. Needed to handle the dynamic font changes needed by the styling engine. Widgets which map surfaces generated by [AG_TextRender()](https://libagar.org/man3/AG_TextRender) should set `USE_TEXT` (and handle "font-changed" events if needed).
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New inheritable [style attributes](https://libagar.org/man3/AG_StyleSheet) _"font-family"_, _"font-size"_, _"font-weight"_ and _"color"_.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New functions [AG_SetFont()](https://libagar.org/man3/AG_SetFont) and [AG_SetStyle()](https://libagar.org/man3/AG_SetStyle) for setting style attributes on a per-widget basis.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New function [AG_RedrawOnChange()](https://libagar.org/man3/AG_RedrawOnChange) to automatically redraw a widget whenever a specified binding value changes.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New function [AG_RedrawOnTick()](https://libagar.org/man3/AG_RedrawOnTick) to redraw a widget periodically at a specified interval in milliseconds.
- [**AG_Window**](https://libagar.org/man3/AG_Window): New option `AG_WINDOW_MAIN`. Break out of [AG_EventLoop](https://libagar.org/man3/AG_EventLoop) once the last window with `MAIN` is destroyed.
- [**AG_Window**](https://libagar.org/man3/AG_Window): New function [AG_WindowSetOpacity()](https://libagar.org/man3/AG_WindowSetOpacity) and `FADEIN` / `FADEOUT` option. Requires a compositing WM.
- [**AG_Window**](https://libagar.org/man3/AG_Window): New function [AG_WindowNewSw()](https://libagar.org/man3/AG_WindowNewSw) for creating windows under specific instances of single-window drivers.
- [**AG_Window**](https://libagar.org/man3/AG_Window): New option `AG_WINDOW_TILING`.
- [**AG_Window**](https://libagar.org/man3/AG_Window): Implement pinning and transient option. New functions [AG_WindowPin()](https://libagar.org/man3/AG_WindowPin) and [AG_WindowMakeTransient()](https://libagar.org/man3/AG_WindowMakeTransient).
- [**AU**](https://libagar.org/man3/AU): Audio interface which also aims at extending _ag_gui_ with sound functionality and audio-related widgets such as waveform-visualization (beta, needs `--enable-au`).
- [**M_String**](https://libagar.org/man3/M_String): New [AG_Printf()](https://libagar.org/man3/AG_Printf) modifiers: `%[R]` (real), `%[T]` (time), `%[C]` (complex number), `%[V]` (vector in R^n), `%[V2]` (vector in R^2), `%[V3]` (vector in R^3), `%[V4]` (vector in R^4), `%[M]` (general matrix) and `%[M44]` (4x4 matrix).
- [**M_Polyhedron**](https://libagar.org/man3/M_Polyhedron): New half-edge based polyhedron element.
- [**M_PointSet**](https://libagar.org/man3/M_PointSet): New element and routines related to sets of points.
- [**M_Matrix**](https://libagar.org/man3/M_Matrix): New SSE versions matrix operations (especially 4x4 matrices).
- [**M_Vector**](https://libagar.org/man3/M_Vector): New SSE versions of some linear-algebra operations.
- [**RG**](https://libagar.org/man3/RG): Imported _ag_rg_ library from the FreeSG project (beta, needs `--enable-rg`).
- [**sdlfb**](https://libagar.org/man3/AG_DriverSDLFB) & [**sdlgl**](https://libagar.org/man3/AG_DriverSDLGL): New function [AG_SetVideoSurfaceSDL()](https://libagar.org/man3/AG_SetVideoSurfaceSDL) for changing the video context at runtime.
- [**cocoa**](https://libagar.org/man3/AG_DriverCocoa): New multi-window driver for MacOS X.
- [**agartest**](https://libagar.org/man1/agartest): New interactive testsuite (in tests/ directory).
- Zoom feature which can be set application-wide or on a per-window basis. Added functions [AG_WindowSetZoom()](https://libagar.org/man3/AG_WindowSetZoom), [AG_ZoomIn()](https://libagar.org/man3/AG_ZoomIn) and [AG_ZoomOut()](https://libagar.org/man3/AG_ZoomOut).
- Introduce generic (_ag_core_ based) framework for low-level event handling. While events and timers were previously handled in _ag_gui_ in a driver-specific fashion, the new interface instead uses the "best" kernel event-notification mechanism available such as [kqueue](https://en.wikipedia.org/wiki/Kqueue), `select()` and `timerfd()`.
- Add support for running multiple [event loops](https://libagar.org/man3/AG_EventLoop) under different threads.
- New function [AG_BindStdGlobalKeys()](https://libagar.org/man3/AG_BindStdGlobalKeys): Set up minimum recommended shortcuts for zoom controls and exiting the application.
- Improved overall efficiency of the GUI rendering pipeline. Removed unnecessary redraw operations. Avoid polling in cases where exclusive access to bound data can be assumed.
- Support for stereographic 3D display modes in GL-based drivers. To request a stereographic visual, one can initialize the driver with the _"stereo"_ option.

### Removed
- [**AG_Button**](https://libagar.org/man3/AG_Button): Remove `MOUSEOVER` code. Mouse is now handled generically by the style engine.
- [**AG_Console**](https://libagar.org/man3/AG_Console): Make [AG_ConsoleSetFont()](https://libagar.org/man3/AG_ConsoleSetFont) deprecated. Fonts are now handled generically by [AG_SetStyle()](https://libagar.org/man3/AG_SetStyle).
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Remove unused `AG_PixelFormat` argument from [AG_ReadSurface()](https://libagar.org/man3/AG_ReadSurface).

### Changed
- Manual pages have been updated. Added example code and images for its web and printable versions.
- [Build system](https://bsdbuild.hypertriton.com) updates. The `agar-config` scripts are now auto-generated.
- [**AG_Combo**](https://libagar.org/man3/AG_Combo): Pin popup menus to parent window. Set `WM_COMBO` hint such that popup menus integrate properly with window managers in multi-window mode.
- [**AG_Config**](https://libagar.org/man3/AG_Config): Save GUI parameters such as keyboard and mouse timings to a configuration file. Define global GUI variables (`agKbdDelay`, etc.) in _ag_gui_ library. The `agConfig` is no longer auto-loaded by [AG_InitCore()](https://libagar.org/man3/AG_InitCore) so user code (and _ag_gui_) now have an opportunity to set up variable bindings before the configuration object is loaded.
- [**AG_Console**](https://libagar.org/man3/AG_Console): Improve scrolling and selection behavior. Make autoscrolling the default.
- [**AG_Console**](https://libagar.org/man3/AG_Console): The [AG_ConsoleAppendLine()](https://libagar.org/man3/AG_ConsoleAppendLine) function may now fail and return NULL.
- [**AG_Console**](https://libagar.org/man3/AG_Console): Add [AG_ConsoleMsgEdit()](https://libagar.org/man3/AG_ConsoleMsgEdit) for updating an existing line. The [agartest](https://libagar.org/man1/agartest) benchmark function is a good demonstration of the feature.
- [**AG_DriverMw**](https://libagar.org/man3/AG_DriverMw): Create a windowless "root" driver instance, so we can handle resources such as X server connections more efficiently.
- [**AG_Label**](https://libagar.org/man3/AG_Label): Polled labels are now implemented using generic [AG_PrintfP()](https://libagar.org/man3/AG_PrintfP) function of [**AG_String**](https://libagar.org/man3/AG_String).
- [**AG_Notebook**](https://libagar.org/man3/AG_Notebook): Display tab labels using embedded [**AG_Label**](https://libagar.org/man3/AG_Label) widgets.
- [**AG_Object**](https://libagar.org/man3/AG_Object): In [AG_ObjectFind()](https://libagar.org/man3/AG_ObjectFind), handle pathnames terminating in "/" correctly.
- [**AG_Pane**](https://libagar.org/man3/AG_Pane): Clean up division/expansion code, replace confusing options with [AG_PaneResizeAction()](https://libagar.org/man3/AG_PaneResizeAction).
- [**AG_Scrollbar**](https://libagar.org/man3/AG_Scrollbar): Handle clicks outside of control bar by seeking progressively.
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Enforce alignment against a 4-byte boundary.
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Honor source X-offset. Thanks pi31415!
- [**AG_Text**](https://libagar.org/man3/AG_Text): The [AG_TextRender()](https://libagar.org/man3/AG_TextRender) call may now fail and return NULL.
- [**AG_Text**](https://libagar.org/man3/AG_Text): Expire entries in the [text cache](https://libagar.org/man3/AG_TextCache) more efficiently.
- [**glx**](https://libagar.org/man3/AG_DriverGLX): Specify an EWMH-compliant window type. Set up Motif functions / decorations / input hints.
- Moved common clipping and texture management code of OpenGL drivers to _gui/drv_gl_common.c_.
- [AG_InitCore()](https://libagar.org/man3/AG_InitCore) now allows a NULL `progname` argument.

### Fixed
- [**AG_Editable**](https://libagar.org/man3/AG_Editable) & [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): In `WORDWRAP` mode, handle additional types of Unicode space in addition to ASCII space.
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): Removed an unnecessary refresh that would occur when the widget was shown initially. Fixed cosmetic problems.
- [**AG_Menu**](https://libagar.org/man3/AG_Menu): Fix modal behavior of [AG_MenuExpand()](https://libagar.org/man3/AG_MenuExpand)-generated popup windows.
- [**AG_Notebook**](https://libagar.org/man3/AG_Notebook): Fix memory leak that would affect container widgets on detach.
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Fixed handling of top-down encoded images in [AG_SurfaceFromBMP()](https://libagar.org/man3/AG_SurfaceFromBMP).
- [**AG_Table**](https://libagar.org/man3/AG_Table): Don't compare cells across columns when restoring selection state in [AG_TableBegin()](https://libagar.org/man3/AG_TableBegin) / [AG_TableEnd()](https://libagar.org/man3/AG_TableEnd).
- [**AG_Text**](https://libagar.org/man3/AG_Text): Fix a thread-safety problem with [AG_TextTmsg()](https://libagar.org/man3/AG_TextTmsg).
- [**AG_Window**](https://libagar.org/man3/AG_Window): Make [AG_WindowShow()](https://libagar.org/man3/AG_WindowShow) and [AG_WindowHide()](https://libagar.org/man3/AG_WindowHide) thread-safe under all drivers. Use queues.
- [**AG_Window**](https://libagar.org/man3/AG_Window): In single-window mode, auto-tile the minimized window icons correctly.
- [**glx**](https://libagar.org/man3/AG_DriverGLX): If Xinerama extension is available, have `GetDisplaySize()` query Xinerama and return the dimensions of the first screen only.
- [**wgl**](https://libagar.org/man3/AG_DriverWGL): Rewrite keyboard handling code. Honor the `AG_WINDOW_KEEPABOVE` option.
- _ag_math_: When compiling the _ag_math_ library with SSE, ignore the configure-specified precision and force `M_Vector[34]`, `M_Matrix44` and `M_Color` to be single-precision.
- Prevent binary structure differences when compiling `--enable-threads` vs `--disable-threads` and `--enable-debug` vs. `--disable-debug`. Thanks reinoud!
- Make the conversion of native surfaces to OpenGL textures more efficient.
- Fix MacOS X build where `_USE_SDL_FRAMEWORK` is set.

## [1.4.1] - 2011-03-20
### Added
- [**AG_Scrollbar**](https://libagar.org/man3/AG_Scrollbar): New option `AUTOHIDE`.
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): New functions [AG_SurfaceFromPNG()](https://libagar.org/man3/AG_SurfaceFromPNG) & [AG_SurfaceFromJPEG()](https://libagar.org/man3/AG_SurfaceFromJPEG). New `AG_Anim` interface.
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): New function [AG_SetJPEGQuality()](https://libagar.org/man3/AG_SetJPEGQuality) to set quality used by [AG_SurfaceExportJPEG](https://libagar.org/man3/AG_SurfaceExportJPEG).
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New functions [AG_Redraw](https://libagar.org/man3/AG_Redraw) to request redraw as soon as possible. New functions [AG_RedrawOnTick](https://libagar.org/man3/AG_RedrawOnTick) to redraw periodically and [AG_RedrawOnChange()](https://libagar.org/man3/AG_RedrawOnChange) to redraw automatically when a given binding value changes. It is recommended for custom event loop routines to check the "dirty" flag of windows in order to avoid unnecessary redrawing and video updates.
- [**AG_Window**](https://libagar.org/man3/AG_Window): Document public [STRUCTURE DATA](https://libagar.org/man3/AG_Window#STRUCTURE_DATA).
- New manual pages: [**AG_DriverGLX**](https://libagar.org/man3/AG_DriverGLX), [**AG_DriverWGL**](https://libagar.org/man3/AG_DriverWGL), [**AG_DriverSDLFB**](https://libagar.org/man3/AG_DriverSDLFB), [**AG_DriverSDLGL**](https://libagar.org/man3/AG_DriverSDLGL), [**AG_DriverMw**](https://libagar.org/man3/AG_DriverMw) & [**AG_DriverSw**](https://libagar.org/man3/AG_DriverSw). Document the available drivers under [**AG_InitGraphics**](https://libagar.org/man3/AG_InitGraphics).
- Port to **Xbox**. Thanks Michael J. Wood!

### Removed
- Removed debug option `--enable-lockdebug`.

### Changed
- [Build system](https://bsdbuild.hypertriton.com) updates: Upgrade to BSDBuild 2.8. Note: stale generated `agar-config/` and `agar-foo-config/` directories from previous builds must be removed before running `./configure`.
- Merged debug options `--enable-classdebug` and `--enable-eventdebug` into a single `--enable-objdebug`.
- Many updates to the manual pages.
- [**wgl**](https://libagar.org/man3/AG_DriverWGL): Terminate the application by breaking out of the event loop as opposed to calling `exit()`.
- [**wgl**](https://libagar.org/man3/AG_DriverWGL): Raise the _"window-gainfocus"_ event whenever a window is focused for the first time on creation.

### Fixed
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): Fixed crash in [AG_SurfaceFromSDL()](https://libagar.org/man3/AG_SurfaceFromSDL). Thanks trapdoor!
- [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): Fixed crash bug with `STATIC` option.
- [**AG_Timeout**](https://libagar.org/man3/AG_Timer): Fix deadlock condition with when timers are scheduled in reverse order. Thanks Jakob Reschke!
- Fixed incorrect UTF-8 conversion of 3-byte sequences. Thanks Jerry Huang!
- Fixed coordinates offset for popup menus under single-window drivers.
- Fixed compilation problems under `--disable-legacy`.
- [**AG_DriverSDLFB**](https://libagar.org/man3/AG_DriverSDLFB) & [**AG_DriverSDLGL**](https://libagar.org/man3/AG_DriverSDLGL): Add required `SDL_LockSurface()` calls.
- Don't use the [gettimeofday()] interface on cygwin.
- Fixed compilation problems under MinGW / MSYS. Thanks Joergen!

## [1.4.0] - 2010-04-16
### Added
- [**AG_Color**](https://libagar.org/man3/AG_Color): Introduce native RGB color type, for cases where we need a representation independent of a given pixel format.
- [**AG_Console**](https://libagar.org/man3/AG_Console): Various improvements. New function `AG_ConsoleSetFont()`.
- [**AG_Driver**](https://libagar.org/man3/AG_Driver): A new framework able to support multi-window systems. The following drivers are provided:
	- [**glx**](https://libagar.org/man3/AG_DriverGLX): GL rendering under X Window System.
	- [**wgl**](https://libagar.org/man3/AG_DriverWGL): GL rendering under Windows.
	- [**sdlgl**](https://libagar.org/man3/AG_DriverSDLGL): GL rendering under [SDL](https://libsdl.org).
	- [**sdlfb**](https://libagar.org/man3/AG_DriverSDLFB): Direct framebuffer rendering via [SDL](https://libsdl.org).
- [**AG_Driver**](https://libagar.org/man3/AG_Driver): Allow new drivers to be implemented as part of an application or external library using a class-registration interface.
- [**AG_Editable**](https://libagar.org/man3/AG_Editable): New function `AG_EditableSetFont()`.
- [**AG_Event**](https://libagar.org/man3/AG_Event): New structure `AG_EventQ` to represent event queues.
- [**AG_File**](https://libagar.org/man3/AG_File): New function [AG_ShortFilename()](https://libagar.org/man3/AG_ShortFilename).
- [**AG_Keyboard**](https://libagar.org/man3/AG_Keyboard) & [**AG_Mouse**](https://libagar.org/man3/AG_Mouse): New interfaces for handling (possibly multiple) input devices. Provides routines to access keyboard and mouse states.
- [**AG_Object**](https://libagar.org/man3/AG_Object): New calls [AG_ObjectSetAttachFn()](https://libagar.org/man3/AG_ObjectSetAttachFn), [AG_ObjectSetDetachFn()](https://libagar.org/man3/AG_ObjectSetDetachFn), [AG_ObjectMoveToHead()](https://libagar.org/man3/AG_ObjectMoveToHead) & [AG_ObjectMoveToTail()](https://libagar.org/man3/AG_ObjectMoveToTail). New hooks allow alternate behavior to be implemented in atomic attach / detach operations. This is used, for example by [**AG_Window**](https://libagar.org/man3/AG_Window) to define the order of windows.
- [**AG_ProgressBar**](https://libagar.org/man3/AG_ProgressBar): New function [AG_ProgressBarSetLength()](https://libagar.org/man3/AG_ProgressBarSetLength).
- [**AG_Scrollbar**](https://libagar.org/man3/AG_Scrollbar): New option `AUTOSIZE`. The control bar is now scaled automatically according to the _"visible"_ binding. New function [AG_ScrollbarSizeHint()](https://libagar.org/man3/AG_ScrollbarSizeHint).
- [**AG_Tbl**](https://libagar.org/man3/AG_Tbl): New hash table of Variables.
- [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): New function `AG_TextboxSetFont()`.
- [**AG_Tlist**](https://libagar.org/man3/AG_Tlist): New functions [AG_TlistUniq()](https://libagar.org/man3/AG_TlistUniq) & [AG_TlistAddHead()](https://libagar.org/man3/AG_TlistAddHead).
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Introduce [Widget Actions](https://libagar.org/man3/AG_Widget#WIDGET_ACTIONS). Offers an abstraction where mouse and keyboard events are mapped to actions by name. The mappings can be reconfigured by the user.
- [**VG**](https://libagar.org/man3/VG): New functions [VG_Status()](https://libagar.org/man3/VG_Status), [VG_TextString()](https://libagar.org/man3/VG_TextString).
- Allow transparency in the default color scheme.
- New C string variants for all functions taking format-string arguments. This allows for more efficient code in many cases, and also avoids complications with language bindings.
- Many additions and improvements to the manual pages.

### Changed
- [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): Embed an [**AG_Label**](https://libagar.org/man3/AG_Label).
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Cursor changes are no longer done by immediate calls in widget code. Instead, widgets now must register _Cursor Change Areas_ through the new [**AG_Cursor**](https://libagar.org/man3/AG_Cursor) interface.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Renamed events _"window-mousemotion"_ -> "_mouse-motion_", _"window-mousebuttonup"_ -> "_mouse-button-up_" and _"window-mousebuttondown"_ -> "_mouse-button-down_".
- [AG_InitGraphics()](https://libagar.org/man3/AG_InitGraphics): New function to initialize the _ag_gui_ library. Deprecate the `AG_InitVideo()` interface. The legacy interface still works, but will select only among single-window drivers.
- It is now possible to build Agar without [SDL](https://libsdl.org) (effectively disabling sdl drivers) using `--without-sdl`.
- The `AG_ViewDetach()` function is now deprecated. Agar windows should be attached with [AG_ObjectAttach()](https://libagar.org/man3/AG_ObjectAttach) (this is done internally) and detached with [AG_ObjectDetach()](https://libagar.org/man3/AG_ObjectDetach).
- [**VG_View**](https://libagar.org/man3/VG_View): Use the new [Widget Actions](https://libagar.org/man3/AG_Widget#WIDGET_ACTIONS) framework.

### Fixed
- [**AG_Variable**](https://libagar.org/man3/AG_Variable): Fix incorrect handling of dynamically-allocated strings.
- Fixed inefficiencies in [AG_LookupClass()](https://libagar.org/man3/AG_LookupClass).
- Don't export our integer types `Uint8`, `Sint8`, `Uint16`, etc. Unwind their definitions when closing headers unless `_USE_AGAR_TYPES` is defined.
- Fixed memory leaks on [AG_Destroy()](https://libagar.org/man3/AG_Destroy). Added test code under `demos/reinit/`.

## [1.3.4] - 2009-08-30
### Added
- [**AG_Box**](https://libagar.org/man3/AG_Box): New function [AG_BoxSetLabel()](https://libagar.org/man3/AG_BoxSetLabel) to display a text caption.
- [**AG_Button**](https://libagar.org/man3/AG_Button): New function [AG_ButtonValign()](https://libagar.org/man3/AG_ButtonValign).
- [**AG_Config**](https://libagar.org/man3/AG_Config): New functions [AG_ConfigLoad()](https://libagar.org/man3/AG_ConfigLoad), [AG_ConfigSave()](https://libagar.org/man3/AG_ConfigSave).
- [**AG_Console**](https://libagar.org/man3/AG_Console): New functions [AG_ConsoleClear()](https://libagar.org/man3/AG_ConsoleClear), [AG_ConsoleMsgIcon()](https://libagar.org/man3/AG_ConsoleMsgIcon) and [AG_ConsoleMsgPtr](https://libagar.org/man3/AG_ConsoleMsgPtr).
- [**AG_DataSource**](https://libagar.org/man3/AG_DataSource): New data source type **AutoCore**. Provides a dynamically-grown memory buffer. New function [AG_OpenAutoCore()](https://libagar.org/man3/AG_OpenAutoCore).
- [**AG_DataSource**](https://libagar.org/man3/AG_DataSource): New function [AG_DataSourceSetErrorFn()](https://libagar.org/man3/AG_DataSourceSetErrorFn).
- [**AG_DSO**](https://libagar.org/man3/AG_DSO): New function [AG_LookupDSO()](https://libagar.org/man3/AG_LookupDSO) to abstract different species of `dlsym()`.
- [**AG_Editable**](https://libagar.org/man3/AG_Editable): Implement word wrapping (`WORDWRAP` option). Thanks to [CoKinetic Systems](http://www.cokinetic.com) for sponsoring this feature!
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): Display OS-specific directory shortcuts. Add shortcut to drive letters on Windows.
- [**AG_FontSelector**](https://libagar.org/man3/AG_FontSelector): Add support for `glob()` pattern matching and autocompletion on platforms where it is available.
- [**AG_FontSelector**](https://libagar.org/man3/AG_FontSelector): New widget for selecting fonts.
- [**AG_GuiDebugger**](https://libagar.org/man3/AG_GuiDebugger): Replaces `DEV_GuiDebugger()` in _ag_dev_.
- [**AG_Object**](https://libagar.org/man3/AG_Object): New function [AG_ObjectReadHeader()](https://libagar.org/man3/AG_ObjectReadHeader) for reading only signature and metadata from an agar object file.
- [**AG_Object**](https://libagar.org/man3/AG_Object): Added debug functionality (type tag for data items) in datafiles. New function [AG_SetSourceDebug()](https://libagar.org/man3/AG_SetSourceDebug). New utility [**agar-disasm**](https://libagar.org/man1/agar-disasm).
- [**AG_Pixmap**](https://libagar.org/man3/AG_Pixmap): New constructor [AG_PixmapFromTexture()](https://libagar.org/man3/AG_PixmapFromTexture).
- [**AG_Pane**](https://libagar.org/man3/AG_Pane): New function [AG_PaneMoveDividerPct()](https://libagar.org/man3/AG_PaneMoveDividerPct) to move the divider to a specified position in "%" of available space. New option `UNMOVABLE`.
- [**AG_Radio**](https://libagar.org/man3/AG_Radio): New constructor [AG_RadioNewUint()](https://libagar.org/man3/AG_RadioNewUint).
- [**AG_Slider**](https://libagar.org/man3/AG_Slider): New constructors `AG_SliderNew*R()`. New function [AG_SliderSetControlSize()](https://libagar.org/man3/AG_SliderSetControlSize).
- [**AG_Table**](https://libagar.org/man3/AG_Table): New functions [AG_TableSetColHeight()](https://libagar.org/man3/AG_TableSetColHeight), [AG_TableSetRowHeight()](https://libagar.org/man3/AG_TableSetRowHeight), [AG_TableSetColMin()](https://libagar.org/man3/AG_TableSetColMin), [AG_TableSetSelectionMode()](https://libagar.org/man3/AG_TableSetSelectionMode) & [AG_TableSetSelectionColor()](https://libagar.org/man3/AG_TableSetSelectionColor). New options `SCROLL_TO_SEL`, `HIGHLIGHT_COLS`. Implemented horizontal scrolling.
- [**AG_Text**](https://libagar.org/man3/AG_Text): New functions [AG_TextFontPts()](https://libagar.org/man3/AG_TextFontPts), [AG_TextFontPct()](https://libagar.org/man3/AG_TextFontPct), [AG_TextFontLookup()](https://libagar.org/man3/AG_TextFontLookup), [AG_TextValign()](https://libagar.org/man3/AG_TextValign).
- [**AG_Text**](https://libagar.org/man3/AG_Text): New ignorable canned dialog [AG_TextInfo()](https://libagar.org/man3/AG_TextInfo).
- [**AG_Time**](https://libagar.org/man3/AG_Time): New interface to monotonically increasing time sources (such as `gettimeofday()`).
- [**AG_Tlist**](https://libagar.org/man3/AG_Tlist): New functions [AG_TlistSetRefresh()](https://libagar.org/man3/AG_TlistSetRefresh), [AG_TlistRefresh()](https://libagar.org/man3/AG_TlistSetRefresh), [AG_TlistScrollToStart()](https://libagar.org/man3/AG_TlistScrollToStart) & [AG_TlistScrollToEnd()](https://libagar.org/man3/AG_TlistScrollToEnd).
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New option flags `NOSPACING` and `TABLE_EMBEDDABLE`. They are advisory flags to parent container widgets.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New function [AG_WidgetIsFocusedInWindow()](https://libagar.org/man3/AG_WidgetIsFocusedInWindow).
- [**AG_WidgetPrimitives**](https://libagar.org/man3/AG_WidgetPrimitives): New functions [AG_DrawArrowLeft()](https://libagar.org/man3/AG_DrawArrowLeft) & [AG_DrawArrowRight()](https://libagar.org/man3/AG_DrawArrowRight). Use stippled polygons in GL mode for _"disabled"_ widget controls.
- [**AG_Window**](https://libagar.org/man3/AG_Window): New function [AG_WindowFocusAtPos()](https://libagar.org/man3/AG_WindowFocusAtPos).
- [**AG_Window**](https://libagar.org/man3/AG_Window): New functions [AG_WindowSetGeometryRect()](https://libagar.org/man3/AG_WindowSetGeometryRect), [AG_WindowSetMinSizePct()](https://libagar.org/man3/AG_WindowSetMinSizePct), [AG_WindowSetSideBorders()](https://libagar.org/man3/AG_WindowSetSideBorders) & [AG_WindowSetBottomBorder()](https://libagar.org/man3/AG_WindowSetBottomBorder).
- [**AG_Variable**](https://libagar.org/man3/AG_Variable): Introduce new interface to represent dynamic runtime properties and references at the [**AG_Object**](https://libagar.org/man3/AG_Object) level (in _ag_core_). The (now deprecated) `AG_WidgetBinding` and `AG_Prop` interfaces continue to emulate the original behavior.
- [**VG**](https://libagar.org/man3/VG): New general-purpose "Select" tool. Adapted from the [edacious](https://edacious.org) editor. Added font selector to "text" element. New function [VG_Merge()](https://libagar.org/man3/VG_Merge).
- [**VG_View**](https://libagar.org/man3/VG_View): New functions [VG_ViewSetScalePreset()](https://libagar.org/man3/VG_ViewSetScalePreset), [VG_ClearEditAreas()](https://libagar.org/man3/VG_ClearEditAreas), [VG_EditNode()](https://libagar.org/man3/VG_EditNode), [VG_DrawSurface()](https://libagar.org/man3/VG_DrawSurface), [VG_ToolCommandExec()](https://libagar.org/man3/VG_ToolCommandExec).
- Updated Perl XS bindings (in p5-Agar/). Thanks Mat Sutcliffe!
- Updated Ada bindings (in ada-gui/, ada-core/ & ada-demos/). Thanks Rothwell!
- New call [AG_InitGUI()](https://libagar.org/man3/AG_InitGUI) to initialize _ag_gui_ without creating any [driver](https://libagar.org/man3/AG_Driver) instance. New function [AG_QuitGUI()](https://libagar.org/man3/AG_QuitGUI) to request a graceful break out of the event loop.
- New `AG_VIDEO_OVERLAY` option to simplify the rendering of Agar GUI elements over an existing OpenGL scene under [custom event loops](https://libagar.org/man3/AG_CustomEventLoop).

### Removed
- Removed build option `--disable-utf8`.
- Removed SDL dependencies in non-driver-specific _ag_gui_ code.

### Changed
- [**AG_GLView**](https://libagar.org/man3/AG_GLView): Save and restore `GL_TRANSFORM_BIT` implicitely.
- [**AG_Timer**](https://libagar.org/man3/AG_Timer): Replace `AG_ScheduleTimeout()` (now deprecated) by new `AG_AddTimeout()` and `AG_ReplaceTimeout()` API.
- [**AG_Treetbl**](https://libagar.org/man3/AG_Treetbl): Updated version of John Blitch's original `AG_Tableview`.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Cache a pointer to the parent [window](https://libagar.org/man3/AG_Window). Make routines such as [AG_WindowUpdate()](https://libagar.org/man3/AG_WindowUpdate) more efficient.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Allow [AG_WindowFocus()](https://libagar.org/man3/AG_WindowFocus) to fail.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New function [AG_WidgetUpdate()](https://libagar.org/man3/AG_WidgetUpdate). Allow widget code to request widget and window redraw asynchronously. This helps avoiding expensive searches in common cases. Use of `AG_WindowUpdate()` within widget code is now deprecated.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New function [AG_WidgetSurface()](https://libagar.org/man3/AG_WidgetSurface) to render GUI widgets to a software [**surface**](https://libagar.org/man3/AG_Surface).
- [**VG**](https://libagar.org/man3/VG): The library can now be built without _ag_gui_. This is useful for command-line tools that must process VG object files.
- Updates to the manual pages.
- [**VG_Text**](https://libagar.org/man3/VG_Text): Implement [Variable](https://libagar.org/man3/AG_Variable) substitution within the text element.

### Fixed
- [**AG_Console**](https://libagar.org/man3/AG_Console): Fixed scrolling behavior.
- [**AG_Label**](https://libagar.org/man3/AG_Label): Fixed clipping when partially occluded.
- [**AG_Menu**](https://libagar.org/man3/AG_Menu): Fixed rendering problems and cosmetic issues.
- [**AG_Pane**](https://libagar.org/man3/AG_Pane): Fixed multiple issues. Fixed the case where [AG_PaneMoveDivider()](https://libagar.org/man3/AG_PaneMoveDivider) was called before the window is first shown. Fixed incorrect documentation of the `DIV` flag. Handle zero-size correctly.
- [**AG_Table**](https://libagar.org/man3/AG_Table): Fixed a bug in the initialization and cleanup of cells containing embedded widgets. Improved handling of out of memory conditions.
- [**AG_Text**](https://libagar.org/man3/AG_Text): Fixed a font engine bug which prevented further reinitialization of Agar after finalization. Thanks Naiina!
- [**AG_Timer**](https://libagar.org/man3/AG_Timer): Avoid unnecessary list traversals when scheduling timeouts.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Fixed incorrect parsing of `<N>px` specifications in [AG_WidgetParseSizeSpec()](https://libagar.org/man3/AG_WidgetParseSizeSpec).
- [**AG_Window**](https://libagar.org/man3/AG_Window): Handle the case where an event handler of a `MODAL` window creates another modal window. We now keep a separate stack of modal windows.
- [**AG_Window**](https://libagar.org/man3/AG_Window): Fixed behavior problems with the focus cycling (tab key) algorithm that could not handle certain container configurations.
- [**AG_Window**](https://libagar.org/man3/AG_Window): Fixed cosmetic problems with decorative window frames and resize controls.
- [**VG**](https://libagar.org/man3/VG): Fixed incorrect rendering of [VG_Polygon](https://libagar.org/man3/VG_Polygon).
- Handle the loss of GL context (which can occur on some platforms following a window resize) by backing mapped [**surface**](https://libagar.org/man3/AG_Surface) textures to software and then restoring them afterwards. It would be nice to be able to test whether this fix is needed or not, as it may impact the performance of window resize operations in multi-window systems.
- Improve handling of out-of-memory conditions.

## [1.3.3] - 2008-10-30
### Added
- [**AG_Checkbox**](https://libagar.org/man3/AG_Checkbox): New bindings `FLAG16` and `FLAG32`.
- [**AG_Combo**](https://libagar.org/man3/AG_Combo) & [**AG_UCombo**](https://libagar.org/man3/AG_UCombo): Added `SCROLLTOSEL` option.
- [**AG_DSO**](https://libagar.org/man3/AG_DSO): New interface for dynamic library loading.
- [**AG_Editable**](https://libagar.org/man3/AG_Editable): New function [AG_EditableSizeHintLines()](https://libagar.org/man3/AG_EditableSizeHintLines).
- [**AG_Graph**](https://libagar.org/man3/AG_Graph): New events _"graph-vertex-selected"_ & "_graph-edge-selected_". New functions [AG_GraphVertexFind()](https://libagar.org/man3/AG_GraphVertexFind), [AG_GraphEdgeFind()](https://libagar.org/man3/AG_GraphEdgeFind), [AG_GraphVertexPopupMenu()](https://libagar.org/man3/AG_GraphVertexPopupMenu) & [AG_GraphEdgePopupMenu()](https://libagar.org/man3/AG_GraphEdgePopupMenu). New options `NO_MOVE`, `NO_SELECT` & `NO_MENUS`.
- [**AG_Label**](https://libagar.org/man3/AG_Label): Allow "polled" label formats to be extended and new specifiers to be registered using [AG_RegisterLabelFormat()](https://libagar.org/man3/AG_RegisterLabelformat). Define `AG_LABEL_ARG()`. New function [AG_LabelValign()](https://libagar.org/man3/AG_LabelValign). Handle specifiers `%lf`, `%lg`, `%llf` & `%llg`. Added some [examples](https://libagar.org/man3/AG_Label#EXAMPLES) to the manual.
- [**AG_Numerical**](https://libagar.org/man3/AG_Numerical): New convenience constructors [AG_NumericalNewTYPE()](https://libagar.org/man3/AG_NumericalNew).
- [**AG_Object**](https://libagar.org/man3/AG_Object): Added support for "namespace" format when specifying classes. For example the string `Agar(Widget:Button):MyButton` would expand to `AG_Object:AG_Widget:AG_Button:MyButton`. New functions [AG_RegisterNamespace()](https://libagar.org/man3/AG_RegisterNamespace) & [AG_UnregisterNamespace()](https://libagar.org/man3/AG_UnregisterNamespace).
- [**AG_Object**](https://libagar.org/man3/AG_Object): Added support for auto-loading DSO modules. New function [AG_LoadClass()](https://libagar.org/man3/AG_LoadClass). Object class specification strings may now include a terminating comma-separated list of modules (`"@module1,module2"`) to be loaded if necessary from one of the [registered module directories](https://libagar.org/man3/AG_RegisterModuleDirectory) by [AG_ObjectLoad()](https://libagar.org/man3/AG_ObjectLoad) whenever loading a serialized Agar object which is an instance of the given class.
- [**AG_Pixmap**](https://libagar.org/man3/AG_Pixmap): New option `RESCALE`. New functions [AG_PixmapReplaceCurrentSurface](https://libagar.org/man3/AG_PixmapReplaceCurrentSurface) & [AG_PixmapUpdateCurrentSurface()](https://libagar.org/man3/AG_PixmapUpdateCurrentSurface).
- [**AG_ProgressBar**](https://libagar.org/man3/AG_ProgressBar): New constructor [AG_ProgressBarNewInt()](https://libagar.org/man3/AG_ProgressBarNewInt).
- [**AG_Radio**](https://libagar.org/man3/AG_Radio): New constructors [AG_RadioNewInt()](https://libagar.org/man3/AG_RadioNewInt) & [AG_RadioNewUint()](https://libagar.org/man3/AG_RadioNewUint).
- [**AG_Scrollview**](https://libagar.org/man3/AG_Scrollview): New scrollable-view container widget.
- [**AG_Surface**](https://libagar.org/man3/AG_Surface): New software surface format.
- [**AG_Rect**](https://libagar.org/man3/AG_Rect): New rectangle structure.
- [**AG_Text**](https://libagar.org/man3/AG_Text): New functions [AG_TextValign()](https://libagar.org/man3/AG_TextValign). New canned dialog [AG_TextError()](https://libagar.org/man3/AG_TextError).
- [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): New function [AG_TextboxSizeHintLines()](https://libagar.org/man3/AG_TextboxSizeHintLines).
- [**AG_Tlist**](https://libagar.org/man3/AG_Tlist): New function [AG_TlistSizeHintLargest()](https://libagar.org/man3/AG_TlistSizeHintLargest) to auto-calculate a default requisition based on the largest item in the set.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Add bindings to floating-point and `[SU]int{8,16,32,64}` types.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Add `rSens` (sensitivity rectangle) field. This enables widgets to mask delivery of events such as _"mouse-button-down"_. The default sensitivity rectangle is the same as the widget area. `rSens` may be modified in `size_allocate()` or `draw()`.
- [**AG_Window**](https://libagar.org/man3/AG_Window): New functions [AG_WindowSetGeometryRect](https://libagar.org/man3/AG_WindowSetGeometryRect), [AG_WindowSetGeometryAligned](https://libagar.org/man3/AG_WindowSetGeometryAligned) & [AG_WindowSetGeometryAlignedPct](https://libagar.org/man3/AG_WindowSetGeometryAlignedPct).
- [**AG_Window**](https://libagar.org/man3/AG_Window): New functions [AG_WindowSetMinSize()](https://libagar.org/man3/AG_WindowSetMinSize), [AG_WindowSetMinSizePct](https://libagar.org/man3/AG_WindowSetMinSizePct), [AG_WindowSelectedWM()](https://libagar.org/man3/AG_WindowSelectedWM) & [AG_WindowIsVisible()](https://libagar.org/man3/AG_WindowIsVisible).
- Imported _ag_math_ library from FreeSG project (beta, needs `--enable-math`).
- New macros `AG_SetCfg<Type>()` and `AG_Cfg<Type>()` to simplify access to global configuration parameters.
- New functions [AG_GetErrorCode()](https://libagar.org/man3/AG_GetErrorCode) & [AG_SetErrorCode()](https://libagar.org/man3/AG_SetErrorCode) for returning numerical error codes.
- New function [AG_SetFatalCallback()](https://libagar.org/man3/AG_SetFatalCallback) for registering a custom routine to call when fatal assertion happens.
- New function [AG_PropDefined()](https://libagar.org/man3/AG_Defined) to check for the existence of an object property.
- New functions [AG_EventInit()](https://libagar.org/man3/AG_EventInit), [AG_EventArgs()](https://libagar.org/man3/AG_EventArgs) & [AG_EventPushTYPE()](https://libagar.org/man3/AG_EventPushPointer) to simplify the process of constructing [**AG_Event**](https://libagar.org/man3/AG_Event) structures.
- New function [AG_InitVideoSDL()](https://libagar.org/man3/AG_InitVideoSDL) for attaching Agar to an existing display [SDL_Surface](https://libsdl.org).
- New convenience routines [AG_Expand()](https://libagar.org/man3/AG_Expand), [AG_ExpandHoriz()](https://libagar.org/man3/AG_ExpandHoriz) & [AG_ExpandVert()](https://libagar.org/man3/AG_ExpandVert). Provides an alternative to passing `HFILL` & `VFILL` flags to constructors.
- New container widget routines [AG_WidgetSetPosition()](https://libagar.org/man3/AG_WidgetSetPosition), [AG_WidgetSetSize()](https://libagar.org/man3/AG_WidgetSetSize) & [AG_WidgetSetGeometry()](https://libagar.org/man3/AG_WidgetSetGeometry).
- New functions [AG_BeginRendering()](https://libagar.org/man3/AG_BeginRendering) & [AG_EndRendering()](https://libagar.org/man3/AG_EndRendering).
- Added [visiblity specifiers](http://gcc.gnu.org/wiki/Visibility) in headers.
- New bindings to the Ada language. Thanks rothwell!

### Changed
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): [AG_WidgetDraw()](https://libagar.org/man3/AG_WidgetDraw) no longer recurses over the child widgets. This allows container widgets the ability to control clipping as well as to tweak the appearance and to define order of rendering of their child widgets.
- [**AG_Window**](https://libagar.org/man3/AG_Window): Make the widths in pixels of side and bottom window borders configurable (applicable to [single-window](https://libagar.org/man3/AG_DriverSw) modes only).
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Improvements to clipping and culling. Clipping is now done according to the intersection of a stack of clipping rectangles. The former `CLIPPING` option flag has been replaced by [AG_PushClipRect()](https://libagar.org/man3/AG_PushClipRect) & [AG_PopClipRect()](https://libagar.org/man3/AG_PopClipRect).
- Don't create application data directories by default unless `AG_CREATE_DATADIR` is passed to [AG_InitCore()](https://libagar.org/man3/AG_InitCore).
- Fixes and improvements to the manual pages. Added EXAMPLE sections. Describe conventions in [**AG_Intro**](https://libagar.org/man3/AG_Intro) [**AG_Threads**](https://libagar.org/man3/AG_Threads).

### Fixed
- [**AG_Table**](https://libagar.org/man3/AG_Table): Fixed problems related to sizing, rendering and event delivery of embedded widgets. Thanks sacrebleu!
- Fixed a build problem with AltiVec under MacOS X 10.4.
- Fixed behavior and appearance of widgets when sizing down to small sizes.

## [1.3.2] - 2008-03-02
### Added
- [**AG_Checkbox**](https://libagar.org/man3/AG_Checkbox): New constructor [AG_CheckboxNewFn()](https://libagar.org/man3/AG_CheckboxNewFn). New option `SET` to force initial binding value to 1.
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): New option `ASYNC` to run callbacks in a separate thread (threaded builds only).
- [**AG_GlobalKeys**](https://libagar.org/man3/AG_GlobalKeys): New function [AG_ClearGlobalKeys()](https://libagar.org/man3/AG_ClearGlobalKeys).
- [**AG_Icon**](https://libagar.org/man3/AG_Icon): New function [AG_IconSetText()](https://libagar.org/man3/AG_IconSetText).
- [**AG_Object**](https://libagar.org/man3/AG_Object): New function [AG_ObjectSuperclass](https://libagar.org/man3/AG_ObjectSuperclass) & [AG_ObjectSetDebugFn()](https://libagar.org/man3/AG_ObjectSetDebugFn).
- [**AG_Scrollbar**](https://libagar.org/man3/AG_Scrollbar): New functions [AG_ScrollbarSetIncFn](https://libagar.org/man3/AG_ScrollbarSetIncFn) & [AG_ScrollbarSetDecFn](https://libagar.org/man3/AG_ScrollbarSetDecFn).
- [**AG_Slider**](https://libagar.org/man3/AG_Slider): New widget for editing a numerical value in a given range.
- [**AG_Socket**](https://libagar.org/man3/AG_Socket): New function [AG_SocketOverlayFn()](https://libagar.org/man3/AG_SocketOverlayFn).
- [**AG_Table**](https://libagar.org/man3/AG_Table): Add support for embedding Agar widgets in the cells of a table. New function [AG_TableSetSeparator()](https://libagar.org/man3/AG_TableSetSeparator).
- [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): Add support for binding to a plain US-ASCII text buffer, for cases where UTF-8 is not desirable. Added [cursor manipulation](https://libagar.org/man3/AG_Textbox#CURSOR_MANIPULATION) routines.
- [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): Handle composition of Latin-1 key sequences.
- [**AG_Window**](https://libagar.org/man3/AG_Window): Implement iconification of Agar windows in [single-window](https://libagar.org/man3/AG_DriverSw) mode. New function [AG_WindowSetIcon()](https://libagar.org/man3/AG_WindowSetIcon). 
- [**AG_Window**](https://libagar.org/man3/AG_Window): New functions [AG_WindowRestoreGeometry](https://libagar.org/man3/AG_WindowRestoreGeometry) & [AG_WindowUnminimize](https://libagar.org/man3/AG_WindowUnminimize). New options `HMAXIMIZE` & `VMAXIMIZE` to preserve window maximization state across resizings of the video display.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Make [AG_SetStyle()](https://libagar.org/man3/AG_SetStyle) changes take effect immediately. New function [AG_WidgetBlitSurfaceFlippedGL()](https://libagar.org/man3/AG_WidgetBlitSurfaceFlippedGL).
- [**AG_WidgetPrimitives**](https://libagar.org/man3/AG_WidgetPrimitives): New function [AG_DrawBoxRounded()](https://libagar.org/man3/AG_DrawBoxRounded).
- New build option `LOCKDEBUG` to enable expensive per-object lock debugging.

### Changed
- [**AG_Editable**](https://libagar.org/man3/AG_Editable): New low-level text edition widget. It implements the functionality formerly in [**AG_Textbox**](https://libagar.org/man3/AG_Textbox). Textbox now works as a proxy which adds a text label, padding and scrollbars in multiline mode.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): In GL mode, maintain the stack of saved clipping planes in the Widget structure itself so we don't need to worry or make any assumptions about GL-implementation-specific limits.

### Fixed
- [**AG_Table**](https://libagar.org/man3/AG_Table): Fixed behavior of scrolling when using a mouse wheel. Fixed issues with keyboard navigation.
- [**AG_Text**](https://libagar.org/man3/AG_Text): Fixed multiline text rendering in [AG_TextRender()](https://libagar.org/man3/AG_TextRender). Thanks Renato Aguiar!
- [**AG_Textbox**](https://libagar.org/man3/AG_Textbox): Fixed problems with multiline text edition.
- [**AG_Titlebar**](https://libagar.org/man3/AG_Titlebar): Fix a bug in the truncation of the caption label.
- [**AG_Tlist**](https://libagar.org/man3/AG_Tlist): Fixed issues with keyboard navigation.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): It is now safe to call [AG_WidgetMapSurface()](https://libagar.org/man3/AG_WidgetMapSurface) from any context or thread. Under GL drivers, use queueing such that the underlying texture upload will be deferred until rendering context.
- Cosmetic fixes in the default theme. Fixed clipping plane off-by-ones in GL mode.
- Fixed compilation problems related to numerical limits on some platforms.
- Fixed compilation problem under MacOS X 10.5.
- Fixed multiple thread-safety problems. It is now safe to combine multiple threads and OpenGL drivers. The big VFS lock has been removed ([AG_LockVFS()](https://libagar.org/man3/AG_LockVFS) now acquires the lock of the VFS root of its argument.
- Fixed the build option `--enable-nls`. Updated French translation.

## [1.3.1] - 2007-12-03
### Fixed
- Fixed compilation issues when compiling under C++.

## [1.3.0] - 2007-11-29
### Added
- [**AG_Button**](https://libagar.org/man3/AG_Button) & [**AG_Checkbox**](https://libagar.org/man3/AG_Checkbox): New _"flags"_ binding, for controlling bits in an integer based on a given bitmask.
- [**AG_Config**](https://libagar.org/man3/AG_Config): New settings _"tmp-path"_ (temporary directory path) and _"initial-run"_ (program runs for the first time).
- [**AG_CPUInfo**](https://libagar.org/man3/AG_CPUInfo): New interface to obtain information about architecture extensions. New function [AG_GetCPUInfo()](https://libagar.org/man3/AG_GetCPUInfo).
- [**AG_DataSource**](https://libagar.org/man3/AG_DataSource): New extensible API to replace the former `AG_Netbuf`.
- [**AG_File**](https://libagar.org/man3/AG_File): New functions [AG_GetSystemTempDir()](https://libagar.org/man3/AG_GetSystemTempDir) & [AG_FileDelete()](https://libagar.org/man3/AG_FileDelete).
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): New functions [AG_FileDlgCheckReadAccess()](https://libagar.org/man3/AG_FileDlgCheckReadAccess) & [AG_FileDlgCheckWriteAccess()](https://libagar.org/man3/AG_FileDlgCheckWriteAccess) for overriding the default behavior of "OK" and "Cancel" buttons.
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): New function [AG_FileDlgSetDirectoryMRU()](https://libagar.org/man3/AG_FileDlgSetDirectoryMRU). Sets the initial directory according to the given persistent MRU parameter.
- [**AG_Label**](https://libagar.org/man3/AG_Label): New option `NOMINSIZE` to display a truncated "..." if the text is partially hidden. Used in [**AG_Titlebar**](https://libagar.org/man3/AG_Titlebar).
- [**AG_Numerical**](https://libagar.org/man3/AG_Numerical): New numerical edition widget which can handle both integer and floating-point bindings. Intended as a replacement for both `AG_FSpinbutton` and `AG_Spinbutton`.
- [**AG_Object**](https://libagar.org/man3/AG_Object): New function [AG_RegisterClass()](https://libagar.org/man3/AG_RegisterClass) to simplify the class-registration process.
- [**AG_Object**](https://libagar.org/man3/AG_Object): New functions `AG_ObjectGetArchivePath()` & `AG_ObjectSetArchivePath()`.
- [**AG_Object**](https://libagar.org/man3/AG_Object): Simpler inheritance mechanics. The object system now implicitely invokes object operations (such as `init()` and `destroy()`) of all parent classes in addition to the current class.
- [**AG_Object**](https://libagar.org/man3/AG_Object): Version checking in [AG_ObjectLoad()](https://libagar.org/man3/AG_ObjectLoad). The version of serialized objects is now compared against that of the registered class. The `load()` operation now accepts an optional [**AG_Version**](https://libagar.org/man3/AG_Version) argument so version differences can be handled programmatically.
- [**AG_Object**](https://libagar.org/man3/AG_Object): [AG_ObjectDestroy()](https://libagar.org/man3/AG_ObjectDestroy) now `free()`s automatically objects that do not have the `STATIC` flag set.
- [**AG_Object**](https://libagar.org/man3/AG_Object): New functions [AG_ObjectLoadGenericFromFile](https://libagar.org/man3/AG_ObjectLoadGenericFromFile) & [AG_ObjectLoadGenericFromFile](https://libagar.org/man3/AG_ObjectLoadGenericFromFile) for loading only the metadata or only the data part of a serialized Agar object.
- [**AG_Object**](https://libagar.org/man3/AG_Object): New option `NAME_ONATTACH` to atomically assign new objects a unique name on attach. New function [AG_ObjectGenName](https://libagar.org/man3/AG_ObjectGenName).
- [**AG_ProgressBar**](https://libagar.org/man3/AG_ProgressBar): New widget which binds to an integer value and range.
- [**AG_Radio**](https://libagar.org/man3/AG_Radio): Add support for hotkeys.
- [**AG_Socket**](https://libagar.org/man3/AG_Socket) & [**AG_Icon**](https://libagar.org/man3/AG_Icon): New widgets for implementing drag-and-droppable icons. See the demo under `demos/sockets/`.
- [**AG_Style**](https://libagar.org/man3/AG_Style): New API for style attributes. New function [AG_SetStyle()](https://libagar.org/man3/AG_SetStyle).
- [**AG_Text**](https://libagar.org/man3/AG_Text): Add support for built-in fonts (fonts bundled into the _ag_gui_ library). This is useful for Windows and other platforms where relying on files is inconvenient.
- [**AG_Text**](https://libagar.org/man3/AG_Text): Use a stackable states to handle rendering attributes such as fonts, colors and justification.
- [**AG_Text**](https://libagar.org/man3/AG_Text): Handle multiline text in [AG_TextRender()](https://libagar.org/man3/AG_TextRender). Added configuration setting "Text Antialiasing".
- [**AG_Text**](https://libagar.org/man3/AG_Text): New canned dialog [AG_TextWarning()](https://libagar.org/man3/AG_TextWarning). Make it ignorable (include a "Don't show again" checkbox).
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): New functions [AG_WidgetEnable()](https://libagar.org/man3/AG_WidgetEnable) & [AG_WidgetDisable()](https://libagar.org/man3/AG_WidgetDisable) to control the active state of widgets. New [AG_WidgetBlitSurfaceGL()](https://libagar.org/man3/AG_WidgetBlitSurfaceGL) routine for blitting to transformed coordinates in GL mode.
- [**AG_Widget**](https://libagar.org/man3/AG_Widget): Introduce distinct `size_request()` and `size_allocate()` operations. `size_request()` is expected to return a preferred initial geometry that the widget would like. The parent container widget invokes `size_allocate()` to let the child widget know the size it was allocated.
- [**MAP**](https://libagar.org/man3/MAP): New object class [**MAP_Actor**](https://libagar.org/man3/MAP_Actor).
- [**RG**](https://libagar.org/man3/RG): Implemented transform chains and caching of transformed items. Added static tile / animation mapping tables, so that external objects can rely on names not changing as items are added or removed.
- [**agarpaint**]: New pixel and animation editor based on the [**RG**](https://libagar.org/man3/RG) framework. It provides a "header export" mode which we can use for icons. Introduce new icon sets, which were all done in agarpaint.
- [**agarrcsd**]: New daemon which implements the server-side of a basic revision control system for serialized _AG_Objects_ (needs `--enable-network`).
- New demos and tests.
- Added "Project files" for various IDEs. They are auto-generated by the [build system](https://bsdbuild.hypertriton.com/man3/build.proj.mk) using [premake](https://premake.github.io) and distributed in the source .zip.
- Added "STRUCTURE DATA" sections to manual pages so we can document public members. Thanks Julien Clement!

### Removed
- Removed the undocumented `AG_HPane` and `AG_VPane` interfaces, both replaced by [**AG_Pane**](https://libagar.org/man3/AG_Pane).
- Removed all remaining graphics-related code from _ag_core_ library.
- Removed ".den" files such as "core-icons.den". Icon resources are now baked in the _ag_gui_ library so programs don't need to load any files on startup.

### Changed
- Upgrade build system to latest [BSDBuild](https://bsdbuild.hypertriton.com) (formerly _csoft-mk_).
- [**AG_Menu**](https://libagar.org/man3/AG_Menu): Store items in a single tree (as opposed to separate trees). Improved handling of dynamic items and boolean states.
- [**AG_Object**](https://libagar.org/man3/AG_Object): Renamed `AG_ObjectOps` -> `AG_ObjectClass`.
- [**AG_Object**](https://libagar.org/man3/AG_Object): `AG_ObjectChanged()` call now performs an exact binary comparison (as opposed to comparing checksums).
- [**AG_Table**](https://libagar.org/man3/AG_Table): Enforce a minimum column width when resizing columns.
- [**AG_Text**](https://libagar.org/man3/AG_Text): Garbage collect entries in the glyph cache using a timestamp method.
- [**AG_Window**](https://libagar.org/man3/AG_Window): Allow more than one `MODAL` window at a time.
- Merged the former _ag_compat_ library into _ag_core_.
- Moved the "Object Manager", monitor and utilities from _ag_gui_ to a separate library called _ag_dev_.
- In the manual, avoid the generic `void *` when referring to Agar objects and refer to the type of the base class instead.

### Fixed
- [**AG_FileDlg**](https://libagar.org/man3/AG_FileDlg): On select platforms use `stat()` to determine the accessibility of a file instead of `open()` which may cause problems with large directories. Thanks KOC!
- [**MAP**](https://libagar.org/man3/MAP): Fix a crash bug when resizing node attribute grids.
- [**RG_Tileview**](https://libagar.org/man3/RG_Tileview): Fix improper alpha-blending of controls under GL modes.
- In [single-window](https://libagar.org/man3/AG_DriverSw) modes, allow windows to move outside of the view area. Thanks Phip!
- Fixed compilation problem if defines such as `HAVE_STDLIB_H` have been set by some header.
- Fixed some off-by-one differences in the way primitives are handled in GL vs. framebuffer modes.
- Fixed compilation under Visual Studio 2005.
- Thread-safety fixes in GL mode. Carefully avoid texture operations outside of rendering context.
- Unwind definitions from Agar header files cleanly.
- Make the build process portable on platforms without symbolic links (such as MSYS).

