" Vim syntax file
" Language:     LibAgar - Agar-GUI C API
" URL:
" https://github.com/JulNadeauCA/libagar/blob/master/syntax/agargui.vim
" Maintainer:   Julien Nadeau Carriere <vedge@csoft.net>
" Last Change:  2023 January 09

" Agar-GUI (https://libagar.org/)
if !exists("c_no_agar_gui") || exists("c_agar_gui_typedefs")
  " gui/box.h
  syn keyword cType AG_Box AG_HBox AG_VBox
  syn keyword cConstant AG_BOX_HORIZ AG_BOX_VERT AG_BOX_TYPE_LAST
  syn keyword cConstant AG_BOX_STYLE_NONE AG_BOX_STYLE_BOX 
  syn keyword cConstant AG_BOX_STYLE_WELL AG_BOX_STYLE_PLAIN
  syn keyword cConstant AG_BOX_STYLE_LAST
  syn keyword cConstant AG_BOX_LEFT AG_BOX_TOP AG_BOX_CENTER AG_BOX_MIDDLE
  syn keyword cConstant AG_BOX_RIGHT AG_BOX_BOTTOM
  syn keyword cConstant AG_BOX_SHADING AG_BOX_NO_SPACING
  syn keyword cConstant AG_BOX_HOMOGENOUS AG_BOX_HFILL AG_BOX_VFILL AG_BOX_EXPAND
  syn keyword cConstant AG_VBOX_HOMOGENOUS AG_VBOX_HFILL AG_VBOX_VFILL AG_VBOX_EXPAND
  syn keyword cConstant AG_HBOX_HOMOGENOUS AG_HBOX_HFILL AG_HBOX_VFILL AG_HBOX_EXPAND
  " gui/button.h
  syn keyword cType AG_Button AG_ButtonRepeat 
  syn keyword cConstant AG_BUTTON_NO_FOCUS AG_BUTTON_STICKY AG_BUTTON_PRESSING
  syn keyword cConstant AG_BUTTON_REPEAT AG_BUTTON_HFILL AG_BUTTON_VFILL
  syn keyword cConstant AG_BUTTON_INVERTED AG_BUTTON_KEYDOWN AG_BUTTON_EXCL
  syn keyword cConstant AG_BUTTON_NOEXCL AG_BUTTON_SET AG_BUTTON_SLOW
  syn keyword cConstant AG_BUTTON_EXPAND
  " gui/checkbox.h
  syn keyword cType AG_Checkbox
  syn keyword cConstant AG_CHECKBOX_HFILL AG_CHECKBOX_VFILL AG_CHECKBOX_SET
  syn keyword cConstant AG_CHECKBOX_INVERT AG_CHECKBOX_EXCL AG_CHECKBOX_EXPAND
  " gui/colors.h
  syn keyword cType AG_Color AG_Grayscale AG_Component AG_ComponentOffset
  syn keyword cType AG_GrayComponent AG_Pixel AG_ColorHSV AG_ColorName
  syn keyword cConstant AG_COMPONENT_BITS AG_COLOR_LAST AG_COLOR_LASTF
  syn keyword cConstant AG_COLOR_LASTD AG_COLOR_PADDING AG_COMPONENT_PADDING
  syn keyword cConstant AG_COLOR_FIRST AG_TRANSPARENT AG_OPAQUE
  syn keyword cConstant AG_SATURATION_EPSILON AG_VALUE_EPSILON
  " gui/combo.h
  syn keyword cType AG_Combo
  syn keyword cConstant AG_COMBO_POLL AG_COMBO_ANY_TEXT AG_COMBO_HFILL
  syn keyword cConstant AG_COMBO_VFILL AG_COMBO_SCROLLTOSEL AG_COMBO_EXPAND
  " gui/console.h
  syn keyword cType AG_ConsoleLine AG_ConsoleFile AG_Console
  syn keyword cConstant AG_CONSOLE_FILE_BINARY AG_CONSOLE_FILE_LEAVE_OPEN
  syn keyword cConstant AG_CONSOLE_HFILL AG_CONSOLE_VFILL AG_CONSOLE_NOAUTOSCROLL
  syn keyword cConstant AG_CONSOLE_NOPOPUP AG_CONSOLE_EXPAND AG_CONSOLE_SELECTING
  syn keyword cConstant AG_CONSOLE_BEGIN_SELECT
  " gui/cursors.h
  syn keyword cType AG_Cursor
  syn keyword cConstant AG_CURSOR_MAX_W AG_CURSOR_MAX_H
  syn keyword cConstant AG_DEFAULT_CURSOR AG_FILL_CURSOR AG_ERASE_CURSOR
  syn keyword cConstant AG_PICK_CURSOR AG_HRESIZE_CURSOR AG_VRESIZE_CURSOR
  syn keyword cConstant AG_LRDIAG_CURSOR AG_LLDIAG_CURSOR AG_TEXT_CURSOR
  syn keyword cConstant AG_LAST_CURSOR
  " gui/dir_dlg.h
  syn keyword cType AG_DirDlg
  syn keyword cConstant AG_DIRDLG_MULTI AG_DIRDLG_CLOSEWIN AG_DIRDLG_LOAD
  syn keyword cConstant AG_DIRDLG_SAVE AG_DIRDLG_RESET_ONSHOW
  syn keyword cConstant AG_DIRDLG_HFILL AG_DIRDLG_VFILL AG_DIRDLG_EXPAND
  syn keyword cConstant AG_DIRDLG_NOBUTTONS
  " gui/drv.h
  syn keyword cType AG_DriverClass AG_Driver AG_DriverEvent AG_DriverEventQ
  syn keyword cConstant AG_FRAMEBUFFER AG_VECTOR AG_WM_SINGLE AG_WM_MULTIPLE
  syn keyword cConstant AG_DRIVER_OPENGL AG_DRIVER_TEXTURES
  syn keyword cConstant AG_DRIVER_SDL1 AG_DRIVER_SDL2 AG_DRIVER_SDL
  syn keyword cConstant AG_DRIVER_WINDOW_BG
  syn keyword cConstant AG_DRIVER_UNKNOWN AG_DRIVER_MOUSE_MOTION
  syn keyword cConstant AG_DRIVER_MOUSE_BUTTON_DOWN AG_DRIVER_MOUSE_BUTTON_UP
  syn keyword cConstant AG_DRIVER_MOUSE_ENTER AG_DRIVER_MOUSE_LEAVE
  syn keyword cConstant AG_DRIVER_FOCUS_IN AG_DRIVER_FOCUS_OUT 
  syn keyword cConstant AG_DRIVER_KEY_DOWN AG_DRIVER_KEY_UP
  syn keyword cConstant AG_DRIVER_EXPOSE AG_DRIVER_VIDEORESIZE AG_DRIVER_CLOSE
  syn keyword cConstant AG_DRIVER_MOVED AG_DRIVER_MINIMIZED AG_DRIVER_MAXIMIZED
  syn keyword cConstant AG_DRIVER_RESTORED AG_DRIVER_SHOWN AG_DRIVER_HIDDEN
  syn keyword cConstant AG_DRIVER_JOY_DEVICE_ADDED AG_DRIVER_JOY_DEVICE_REMOVED
  syn keyword cConstant AG_DRIVER_JOY_AXIS_MOTION AG_DRIVER_JOY_HAT_MOTION
  syn keyword cConstant AG_DRIVER_JOY_BALL_MOTION AG_DRIVER_JOY_BUTTON_DOWN
  syn keyword cConstant AG_DRIVER_JOY_BUTTON_UP AG_DRIVER_EVENT_LAST
  " gui/drv_dummy.c
  syn keyword cType AG_DriverDUMMY AG_CursorDUMMY
  " gui/drv_cocoa.m
  syn keyword cType AG_DriverCocoa AG_CocoaWindow AG_CocoaListener
  " gui/drv_gl_common.c
  syn keyword cType AG_GL_BlendState AG_GL_Context
  " gui/drv_glx.c
  syn keyword cType AG_DriverGLX AG_CursorGLX
  " gui/drv_mw.h
  syn keyword cType AG_DriverMwClass AG_DriverMw
  syn keyword cConstant AG_DRIVER_MW_OPEN AG_DRIVER_MW_ANYPOS AG_DRIVER_MW_ANYPOS_AVAIL
  " gui/drv_sdl2fb.c
  syn keyword cType AG_DriverSDL2FB
  " gui/drv_sdl2gl.c
  syn keyword cType AG_DriverSDL2GL
  " gui/drv_sdl2mw.c
  syn keyword cType AG_DriverSDL2MW
  " gui/drv_sdlfb.c
  syn keyword cType AG_DriverSDLFB
  " gui/drv_sdlgl.c
  syn keyword cType AG_DriverSDLGL
  " gui/drv_sw.h
  syn keyword cType AG_DriverSwClass AG_DriverSw
  syn keyword cConstant AG_WINDOW_ALIGNMENT_NONE AG_WINDOW_ALIGNMENT_LAST
  syn keyword cConstant AG_WINDOW_TL AG_WINDOW_TC AG_WINDOW_TR 
  syn keyword cConstant AG_WINDOW_ML AG_WINDOW_MC AG_WINDOW_MR
  syn keyword cConstant AG_WINDOW_BL AG_WINDOW_BC AG_WINDOW_BR
  syn keyword cConstant AG_WINOP_NONE AG_WINOP_MOVE AG_WINOP_LRESIZE
  syn keyword cConstant AG_WINOP_RRESIZE AG_WINOP_HRESIZE
  syn keyword cConstant AG_DRIVER_SW_OVERLAY AG_DRIVER_SW_BGPOPUP
  syn keyword cConstant AG_DRIVER_SW_FULLSCREEN AG_DRIVER_SW_REDRAW
  " gui/drv_wgl.c
  syn keyword cType AG_DriverWGL AG_CursorWGL
  " gui/editable.h
  syn keyword cType AG_EditableBuffer AG_EditableRevision AG_EditableClipboard
  syn keyword cType AG_Autocomplete AG_Editable
  syn keyword cConstant AG_EDITABLE_MAX AG_EDITABLE_HFILL AG_EDITABLE_VFILL
  syn keyword cConstant AG_EDITABLE_EXPAND AG_EDITABLE_MULTILINE
  syn keyword cConstant AG_EDITABLE_BLINK_ON AG_EDITABLE_PASSWORD
  syn keyword cConstant AG_EDITABLE_ABANDON_FOCUS AG_EDITABLE_INT_ONLY
  syn keyword cConstant AG_EDITABLE_FLT_ONLY AG_EDITABLE_CATCH_TAB
  syn keyword cConstant AG_EDITABLE_CURSOR_MOVING AG_EDITABLE_UPPERCASE
  syn keyword cConstant AG_EDITABLE_KEEPVISCURSOR AG_EDITABLE_LOWERCASE
  syn keyword cConstant AG_EDITABLE_MARKPREF AG_EDITABLE_EXCL AG_EDITABLE_NO_KILL_YANK
  syn keyword cConstant AG_EDITABLE_RETURN_HELD AG_EDITABLE_NO_ALT_LATIN1
  syn keyword cConstant AG_EDITABLE_WORDWRAP AG_EDITABLE_NOPOPUP
  syn keyword cConstant AG_EDITABLE_WORDSELECT AG_EDITABLE_READONLY
  syn keyword cConstant AG_EDITABLE_MULTILINGUAL AG_EDITABLE_SHIFT_SELECT
  " gui/file_dlg.h
  syn keyword cType AG_FileDlg AG_FileType AG_FileOption
  syn keyword cConstant AG_FILEDLG_BOOL AG_FILEDLG_INT AG_FILEDLG_FLOAT
  syn keyword cConstant AG_FILEDLG_DOUBLE AG_FILEDLG_STRING
  syn keyword cConstant AG_FILE_TYPE_SELECTED
  syn keyword cConstant AG_FILEDLG_MULTI AG_FILEDLG_CLOSEWIN AG_FILEDLG_LOAD
  syn keyword cConstant AG_FILEDLG_SAVE AG_FILEDLG_ASYNC AG_FILEDLG_RESET_ONSHOW
  syn keyword cConstant AG_FILEDLG_NOBUTTONS AG_FILEDLG_MASK_EXT
  syn keyword cConstant AG_FILEDLG_MASK_HIDDEN AG_FILEDLG_NOMASKOPTS
  syn keyword cConstant AG_FILEDLG_NOTYPESELECT AG_FILEDLG_COMPACT
  syn keyword cConstant AG_FILEDLG_HFILL AG_FILEDLG_VFILL AG_FILEDLG_EXPAND
  syn keyword cConstant AG_FILEDLG_INHERITED_FLAGS
  " gui/fixed.h
  syn keyword cType AG_Fixed
  syn keyword cConstant AG_FIXED_STYLE_NONE AG_FIXED_STYLE_BOX
  syn keyword cConstant AG_FIXED_STYLE_WELL AG_FIXED_STYLE_PLAIN
  syn keyword cConstant AG_FIXED_STYLE_LAST
  syn keyword cConstant AG_FIXED_HFILL AG_FIXED_VFILL
  syn keyword cConstant AG_FIXED_NO_UPDATE AG_FIXED_EXPAND
  " gui/fixed_plotter.h
  syn keyword cType AG_FixedPlotter AG_FixedPlotterItem AG_FixedPlotterValue
  syn keyword cConstant AG_FIXED_PLOTTER_LABEL_MAX AG_FIXED_PLOTTER_POINTS
  syn keyword cConstant AG_FIXED_PLOTTER_LINES AG_FIXED_PLOTTER_SCROLL
  syn keyword cConstant AG_FIXED_PLOTTER_XAXIS AG_FIXED_PLOTTER_HFILL
  syn keyword cConstant AG_FIXED_PLOTTER_VFILL AG_FIXED_PLOTTER_EXPAND
  " gui/font_selector.h
  syn keyword cType AG_FontSelector
  syn keyword cConstant AG_FONTSELECTOR_UPDATE AG_FONTSELECTOR_ALT_PHRASE
  syn keyword cConstant AG_FONTSELECTOR_SW_STYLES AG_FONTSELECTOR_OBLIQUE_STYLES
  syn keyword cConstant AG_FONTSELECTOR_HFILL AG_FONTSELECTOR_VFILL
  syn keyword cConstant AG_FONTSELECTOR_EXPAND
  " gui/geometry.h
  syn keyword cType AG_Pt AG_Rect AG_Rect2 AG_ClipRect AG_TexCoord
  " gui/glview.h
  syn keyword cType AG_GLView
  syn keyword cConstant AG_GLVIEW_HFILL AG_GLVIEW_VFILL AG_GLVIEW_NOMODELVIEW
  syn keyword cConstant AG_GLVIEW_NOTEXTURE AG_GLVIEW_NOCOLOR AG_GLVIEW_INIT_MATRICES
  syn keyword cConstant AG_GLVIEW_RESHAPE AG_GLVIEW_BGFILL AG_GLVIEW_EXPAND
  " gui/graph.h
  syn keyword cType AG_Graph AG_GraphVertex AG_GraphEdge
  syn keyword cConstant AG_GRAPH_LABEL_MAX AG_GRAPH_RECTANGLE AG_GRAPH_CIRCLE
  syn keyword cConstant AG_GRAPH_EDGE_UNDIRECTED AG_GRAPH_EDGE_DIRECTED
  syn keyword cConstant AG_GRAPH_MOUSEOVER AG_GRAPH_SELECTED AG_GRAPH_HIDDEN
  syn keyword cConstant AG_GRAPH_AUTOPLACED AG_GRAPH_HFILL AG_GRAPH_VFILL
  syn keyword cConstant AG_GRAPH_EXPAND AG_GRAPH_SCROLL AG_GRAPH_DRAGGING
  syn keyword cConstant AG_GRAPH_PANNING AG_GRAPH_NO_MOVE AG_GRAPH_NO_SELECT
  syn keyword cConstant AG_GRAPH_READONLY
  " gui/gui.h
  syn keyword cConstant AG_ZOOM_MIN AG_ZOOM_MAX AG_ZOOM_1_1 AG_ZOOM_DEFAULT
  syn keyword cConstant AG_VIDEO_HWSURFACE AG_VIDEO_ASYNCBLIT AG_VIDEO_ANYFORMAT
  syn keyword cConstant AG_VIDEO_HWPALETTE AG_VIDEO_DOUBLEBUF AG_VIDEO_FULLSCREEN
  syn keyword cConstant AG_VIDEO_RESIZABLE AG_VIDEO_NOFRAME AG_VIDEO_BORDERLESS
  syn keyword cConstant AG_VIDEO_BGPOPUPMENU AG_VIDEO_OPENGL AG_VIDEO_OPENGL_OR_SDL
  syn keyword cConstant AG_VIDEO_NOBGCLEAR AG_VIDEO_OVERLAY AG_VIDEO_SDL
  syn keyword cConstant AG_VIDEO_FIXED
  " gui/gui_math.h
  syn keyword cConstant AG_PI
  " gui/hsvpal.h
  syn keyword cType AG_HSVPal
  syn keyword cConstant AG_HSVPAL_PIXEL AG_HSVPAL_DIRTY AG_HSVPAL_HFILL
  syn keyword cConstant AG_HSVPAL_VFILL AG_HSVPAL_NOALPHA AG_HSVPAL_FORCE_NOALPHA
  syn keyword cConstant AG_HSVPAL_NOPREVIEW AG_HSVPAL_SHOW_RGB
  syn keyword cConstant AG_HSVPAL_SHOW_HSV AG_HSVPAL_SHOW_RGB_HSV
  syn keyword cConstant AG_HSVPAL_EXPAND AG_HSVPAL_SEL_NONE AG_HSVPAL_SEL_H
  syn keyword cConstant AG_HSVPAL_SEL_SV AG_HSVPAL_SEL_A
  " gui/icon.h
  syn keyword cType AG_Icon
  syn keyword cConstant AG_ICON_REGEN_LABEL AG_ICON_DND AG_ICON_DBLCLICKED
  syn keyword cConstant AG_ICON_BGFILL
  " gui/iconmgr.h
  syn keyword cType AG_StaticIcon
  " gui/input_device.h
  syn keyword cType AG_InputDevice
  syn keyword cType AG_Controller AG_ControllerEvent AG_ControllerEventType
  syn keyword cType AG_ControllerButton AG_TouchEvent AG_TouchEventType
  syn keyword cConstant AG_TOUCH_FINGER_MOTION AG_TOUCH_FINGER_DOWN
  syn keyword cConstant AG_TOUCH_FINGER_UP AG_TOUCH_MULTIGESTURE
  syn keyword cConstant AG_TOUCH_DOLLAR_GESTURE AG_TOUCH_DOLLAR_RECORD
  syn keyword cConstant AG_TOUCH_EVENT_LAST
  syn keyword cConstant AG_CONTROLLER_BUTTON_INVALID AG_CONTROLLER_BUTTON_A
  syn keyword cConstant AG_CONTROLLER_BUTTON_B AG_CONTROLLER_BUTTON_X
  syn keyword cConstant AG_CONTROLLER_BUTTON_Y AG_CONTROLLER_BUTTON_BACK
  syn keyword cConstant AG_CONTROLLER_BUTTON_GUIDE AG_CONTROLLER_BUTTON_START
  syn keyword cConstant AG_CONTROLLER_BUTTON_LEFT_STICK AG_CONTROLLER_BUTTON_RIGHT_STICK
  syn keyword cConstant AG_CONTROLLER_BUTTON_LEFT_SHOULDER AG_CONTROLLER_BUTTON_RIGHT_SHOULDER
  syn keyword cConstant AG_CONTROLLER_BUTTON_DPAD_UP AG_CONTROLLER_BUTTON_DPAD_DOWN
  syn keyword cConstant AG_CONTROLLER_BUTTON_DPAD_LEFT AG_CONTROLLER_BUTTON_DPAD_RIGHT
  syn keyword cConstant AG_CONTROLLER_BUTTON_MISC1 AG_CONTROLLER_BUTTON_PADDLE1
  syn keyword cConstant AG_CONTROLLER_BUTTON_PADDLE2 AG_CONTROLLER_BUTTON_PADDLE3
  syn keyword cConstant AG_CONTROLLER_BUTTON_PADDLE4 AG_CONTROLLER_BUTTON_TOUCHPAD
  syn keyword cConstant AG_CONTROLLER_BUTTON_MAX AG_CONTROLLER_BUTTON_LAST
  syn keyword cConstant AG_CONTROLLER_AXIS_MOTION AG_CONTROLLER_BUTTON_DOWN
  syn keyword cConstant AG_CONTROLLER_BUTTON_UP AG_CONTROLLER_DEVICE_ADDED
  syn keyword cConstant AG_CONTROLLER_DEVICE_REMOVED AG_CONTROLLER_DEVICE_REMAPPED
  syn keyword cConstant AG_CONTROLLER_TOUCHPAD AG_CONTROLLER_SENSOR
  syn keyword cConstant AG_CONTROLLER_EVENT_LAST
  " gui/joystick.h
  syn keyword cType AG_Joystick AG_JoystickType AG_JoyEvent AG_JoyEventType
  syn keyword cConstant AG_JOY_AXIS_MOTION AG_JOY_BALL_MOTION AG_JOY_HAT_MOTION
  syn keyword cConstant AG_JOY_BUTTON AG_JOY_DEVICE AG_JOY_BATTERY AG_JOY_EVENT_LAST
  syn keyword cConstant AG_JOYSTICK_TYPE_UNKNOWN AG_JOYSTICK_TYPE_GAMECONTROLLER
  syn keyword cConstant AG_JOYSTICK_TYPE_WHEEL AG_JOYSTICK_TYPE_ARCADE_STICK
  syn keyword cConstant AG_JOYSTICK_TYPE_FLIGHT_STICK AG_JOYSTICK_TYPE_DANCE_PAD
  syn keyword cConstant AG_JOYSTICK_TYPE_GUITAR AG_JOYSTICK_TYPE_DRUM_KIT
  syn keyword cConstant AG_JOYSTICK_TYPE_ARCADE_PAD AG_JOYSTICK_TYPE_THROTTLE
  syn keyword cConstant AG_JOYSTICK_TYPE_LAST
  syn keyword cConstant AG_JOYSTICK_HAS_LED AG_JOYSTICK_HAS_RUMBLE
  syn keyword cConstant AG_JOYSTICK_HAS_RUMBLE_TRIGGERS
  " gui/keyboard.h
  syn keyword cType AG_Keyboard AG_KeyCategory AG_Key AG_KeySym AG_KeyMod
  syn keyword cType AG_KeyboardAction
  syn keyword cConstant AG_KEY_PRESSED AG_KEY_RELEASED
  syn keyword cConstant AG_KCAT_NONE AG_KCAT_CONTROL AG_KCAT_SPACING
  syn keyword cConstant AG_KCAT_RETURN AG_KCAT_PRINT AG_KCAT_ALPHA AG_KCAT_NUMBER
  syn keyword cConstant AG_KCAT_DIR AG_KCAT_FUNCTION AG_KCAT_LOCK AG_KCAT_MODIFIER
  syn keyword cConstant AG_KCAT_LAST
  syn keyword cConstant AG_KEY_NONE AG_KEY_ASCII_START AG_KEY_BACKSPACE
  syn keyword cConstant AG_KEY_TAB AG_KEY_CLEAR AG_KEY_RETURN AG_KEY_PAUSE
  syn keyword cConstant AG_KEY_ESCAPE AG_KEY_SPACE AG_KEY_EXCLAIM AG_KEY_QUOTEDBL
  syn keyword cConstant AG_KEY_HASH AG_KEY_DOLLAR AG_KEY_PERCENT AG_KEY_AMPERSAND
  syn keyword cConstant AG_KEY_QUOTE AG_KEY_LEFTPAREN AG_KEY_RIGHTPAREN
  syn keyword cConstant AG_KEY_ASTERISK AG_KEY_PLUS AG_KEY_COMMA AG_KEY_MINUS
  syn keyword cConstant AG_KEY_PERIOD AG_KEY_SLASH
  syn keyword cConstant AG_KEY_0 AG_KEY_1 AG_KEY_2 AG_KEY_3 AG_KEY_4
  syn keyword cConstant AG_KEY_5 AG_KEY_6 AG_KEY_7 AG_KEY_8 AG_KEY_9
  syn keyword cConstant AG_KEY_COLON AG_KEY_SEMICOLON AG_KEY_LESS AG_KEY_EQUALS
  syn keyword cConstant AG_KEY_GREATER AG_KEY_QUESTION AG_KEY_AT
  syn keyword cConstant AG_KEY_LEFTBRACKET AG_KEY_BACKSLASH AG_KEY_RIGHTBRACKET
  syn keyword cConstant AG_KEY_CARET AG_KEY_UNDERSCORE AG_KEY_BACKQUOTE
  syn keyword cConstant AG_KEY_A AG_KEY_B AG_KEY_C AG_KEY_D AG_KEY_E AG_KEY_F
  syn keyword cConstant AG_KEY_G AG_KEY_H AG_KEY_I AG_KEY_J AG_KEY_K AG_KEY_L
  syn keyword cConstant AG_KEY_M AG_KEY_N AG_KEY_O AG_KEY_P AG_KEY_Q AG_KEY_R
  syn keyword cConstant AG_KEY_S AG_KEY_T AG_KEY_U AG_KEY_V AG_KEY_W AG_KEY_X
  syn keyword cConstant AG_KEY_Y AG_KEY_Z AG_KEY_DELETE AG_KEY_ASCII_END
  syn keyword cConstant AG_KEY_KP0 AG_KEY_KP1 AG_KEY_KP2 AG_KEY_KP3 AG_KEY_KP4
  syn keyword cConstant AG_KEY_KP5 AG_KEY_KP6 AG_KEY_KP7 AG_KEY_KP8 AG_KEY_KP9
  syn keyword cConstant AG_KEY_KP_PERIOD AG_KEY_KP_DIVIDE AG_KEY_KP_MULTIPLY
  syn keyword cConstant AG_KEY_KP_MINUS AG_KEY_KP_PLUS AG_KEY_KP_ENTER
  syn keyword cConstant AG_KEY_KP_EQUALS AG_KEY_UP AG_KEY_DOWN AG_KEY_RIGHT
  syn keyword cConstant AG_KEY_LEFT AG_KEY_INSERT AG_KEY_HOME AG_KEY_END
  syn keyword cConstant AG_KEY_PAGEUP AG_KEY_PAGEDOWN AG_KEY_F1 AG_KEY_F2
  syn keyword cConstant AG_KEY_F3 AG_KEY_F4 AG_KEY_F5 AG_KEY_F6 AG_KEY_F7
  syn keyword cConstant AG_KEY_F8 AG_KEY_F9 AG_KEY_F10 AG_KEY_F11 AG_KEY_F12
  syn keyword cConstant AG_KEY_F13 AG_KEY_F14 AG_KEY_F15 AG_KEY_NUMLOCK
  syn keyword cConstant AG_KEY_CAPSLOCK AG_KEY_SCROLLOCK AG_KEY_RSHIFT
  syn keyword cConstant AG_KEY_LSHIFT AG_KEY_RCTRL AG_KEY_LCTRL AG_KEY_RALT
  syn keyword cConstant AG_KEY_LALT AG_KEY_RMETA AG_KEY_LMETA AG_KEY_LSUPER
  syn keyword cConstant AG_KEY_RSUPER AG_KEY_MODE AG_KEY_COMPOSE AG_KEY_HELP
  syn keyword cConstant AG_KEY_PRINT AG_KEY_SYSREQ AG_KEY_BREAK AG_KEY_MENU
  syn keyword cConstant AG_KEY_POWER AG_KEY_EURO AG_KEY_UNDO AG_KEY_GRAVE
  syn keyword cConstant AG_KEY_KP_CLEAR AG_KEY_COMMAND AG_KEY_FUNCTION
  syn keyword cConstant AG_KEY_VOLUME_UP AG_KEY_VOLUME_DOWN AG_KEY_VOLUME_MUTE
  syn keyword cConstant AG_KEY_F16 AG_KEY_F17 AG_KEY_F18 AG_KEY_F19 AG_KEY_F20
  syn keyword cConstant AG_KEY_F21 AG_KEY_F22 AG_KEY_F23 AG_KEY_F24 AG_KEY_F25
  syn keyword cConstant AG_KEY_F26 AG_KEY_F27 AG_KEY_F28 AG_KEY_F29 AG_KEY_F30
  syn keyword cConstant AG_KEY_F31 AG_KEY_F32 AG_KEY_F33 AG_KEY_F34 AG_KEY_F35
  syn keyword cConstant AG_KEY_BEGIN AG_KEY_RESET AG_KEY_STOP AG_KEY_USER
  syn keyword cConstant AG_KEY_SYSTEM AG_KEY_PRINT_SCREEN AG_KEY_CLEAR_LINE
  syn keyword cConstant AG_KEY_CLEAR_DISPLAY AG_KEY_INSERT_LINE AG_KEY_DELETE_LINE
  syn keyword cConstant AG_KEY_INSERT_CHAR AG_KEY_DELETE_CHAR AG_KEY_PREV
  syn keyword cConstant AG_KEY_NEXT AG_KEY_SELECT AG_KEY_EXECUTE AG_KEY_REDO
  syn keyword cConstant AG_KEY_FIND AG_KEY_MODE_SWITCH AG_KEY_NON_US_BACKSLASH
  syn keyword cConstant AG_KEY_APPLICATION AG_KEY_AGAIN AG_KEY_CUT AG_KEY_PASTE
  syn keyword cConstant AG_KEY_KP_COMMA AG_KEY_KP_EQUALS_AS_400
  syn keyword cConstant AG_KEY_INTERNATIONAL_1 AG_KEY_INTERNATIONAL_2
  syn keyword cConstant AG_KEY_INTERNATIONAL_3 AG_KEY_INTERNATIONAL_4
  syn keyword cConstant AG_KEY_INTERNATIONAL_5 AG_KEY_INTERNATIONAL_6
  syn keyword cConstant AG_KEY_INTERNATIONAL_7 AG_KEY_INTERNATIONAL_8
  syn keyword cConstant AG_KEY_INTERNATIONAL_9
  syn keyword cConstant AG_KEY_LANGUAGE_1 AG_KEY_LANGUAGE_2 AG_KEY_LANGUAGE_3
  syn keyword cConstant AG_KEY_LANGUAGE_4 AG_KEY_LANGUAGE_5 AG_KEY_LANGUAGE_6
  syn keyword cConstant AG_KEY_LANGUAGE_7 AG_KEY_LANGUAGE_8 AG_KEY_LANGUAGE_9
  syn keyword cConstant AG_KEY_ALT_ERASE AG_KEY_CANCEL AG_KEY_PRIOR AG_KEY_RETURN2
  syn keyword cConstant AG_KEY_SEPARATOR AG_KEY_OUT AG_KEY_OPER AG_KEY_CLEAR_AGAIN
  syn keyword cConstant AG_KEY_CRSEL AG_KEY_EXSEL AG_KEY_KP_00 AG_KEY_KP_000
  syn keyword cConstant AG_KEY_THOUSANDS_SEPARATOR AG_KEY_DECIMALS_SEPARATOR
  syn keyword cConstant AG_KEY_CURRENCY_UNIT AG_KEY_CURRENCY_SUBUNIT
  syn keyword cConstant AG_KEY_KP_LEFT_PAREN AG_KEY_KP_RIGHT_PAREN
  syn keyword cConstant AG_KEY_KP_LEFT_BRACE AG_KEY_KP_RIGHT_BRACE
  syn keyword cConstant AG_KEY_KP_TAB AG_KEY_KP_BACKSPACE
  syn keyword cConstant AG_KEY_KP_A AG_KEY_KP_B AG_KEY_KP_C AG_KEY_KP_D
  syn keyword cConstant AG_KEY_KP_E AG_KEY_KP_F AG_KEY_KP_XOR AG_KEY_KP_POWER
  syn keyword cConstant AG_KEY_KP_PERCENT AG_KEY_KP_LESS AG_KEY_KP_GREATER
  syn keyword cConstant AG_KEY_KP_AMPERSAND AG_KEY_KP_DBL_AMPERSAND
  syn keyword cConstant AG_KEY_KP_VERTICAL_BAR AG_KEY_KP_DBL_VERTICAL_BAR
  syn keyword cConstant AG_KEY_KP_COLON AG_KEY_KP_HASH AG_KEY_KP_SPACE
  syn keyword cConstant AG_KEY_KP_AT AG_KEY_KP_EXCLAM AG_KEY_KP_MEM_STORE
  syn keyword cConstant AG_KEY_KP_MEM_RECALL AG_KEY_KP_MEM_CLEAR
  syn keyword cConstant AG_KEY_KP_MEM_ADD AG_KEY_KP_MEM_SUBTRACT
  syn keyword cConstant AG_KEY_KP_MEM_MULTIPLY AG_KEY_KP_MEM_DIVIDE
  syn keyword cConstant AG_KEY_KP_PLUS_MINUS AG_KEY_KP_CLEAR_ENTRY
  syn keyword cConstant AG_KEY_KP_BINARY AG_KEY_KP_OCTAL AG_KEY_KP_DECIMAL
  syn keyword cConstant AG_KEY_KP_HEXADECIMAL AG_KEY_LGUI AG_KEY_RGUI
  syn keyword cConstant AG_KEY_AUDIO_NEXT AG_KEY_AUDIO_PREV AG_KEY_AUDIO_STOP
  syn keyword cConstant AG_KEY_AUDIO_PLAY AG_KEY_AUDIO_MUTE AG_KEY_MEDIA_SELECT
  syn keyword cConstant AG_KEY_WWW AG_KEY_MAIL AG_KEY_CALCULATOR AG_KEY_COMPUTER
  syn keyword cConstant AG_KEY_AC_SEARCH AG_KEY_AC_HOME AG_KEY_AC_BACK
  syn keyword cConstant AG_KEY_AC_FORWARD AG_KEY_AC_STOP AG_KEY_AC_REFRESH
  syn keyword cConstant AG_KEY_AC_BOOKMARKS AG_KEY_AUDIO_REWIND
  syn keyword cConstant AG_KEY_AUDIO_FASTFORWARD AG_KEY_LAST AG_KEY_ANY
  syn keyword cConstant AG_KEYMOD_NONE AG_KEYMOD_LSHIFT AG_KEYMOD_RSHIFT
  syn keyword cConstant AG_KEYMOD_CTRL_SHIFT AG_KEYMOD_CTRL_ALT
  syn keyword cConstant AG_KEYMOD_LCTRL AG_KEYMOD_RCTRL
  syn keyword cConstant AG_KEYMOD_LALT AG_KEYMOD_RALT
  syn keyword cConstant AG_KEYMOD_LMETA AG_KEYMOD_RMETA
  syn keyword cConstant AG_KEYMOD_NUMLOCK AG_KEYMOD_CAPSLOCK AG_KEYMOD_MODE
  syn keyword cConstant AG_KEYMOD_CTRL AG_KEYMOD_SHIFT AG_KEYMOD_ALT
  syn keyword cConstant AG_KEYMOD_META AG_KEYMOD_ANY
  " gui/label.h
  syn keyword cType AG_Label
  syn keyword cConstant AG_LABEL_MAX AG_LABEL_MAX_POLLPTRS AG_SMALL_LABEL_MAX
  syn keyword cConstant AG_LABEL_STATIC AG_LABEL_POLLED AG_LABEL_TYPE_LAST
  syn keyword cConstant AG_LABEL_HFILL AG_LABEL_VFILL AG_LABEL_PARTIAL
  syn keyword cConstant AG_LABEL_REGEN AG_LABEL_FRAME AG_LABEL_SLOW
  syn keyword cConstant AG_LABEL_EXPAND
  " gui/menu.h
  syn keyword cType AG_Menu AG_MenuItem AG_MenuView AG_PopupMenu
  syn keyword cConstant AG_MENU_ITEM_TAG AG_MENU_ITEM_TAG_LEN
  syn keyword cConstant AG_MENU_ITEM_ICONS AG_MENU_ITEM_NOSELECT
  syn keyword cConstant AG_MENU_ITEM_SEPARATOR AG_MENU_ITEM_INVERTED
  syn keyword cConstant AG_MENU_NO_BINDING AG_MENU_INT_BOOL AG_MENU_INT8_BOOL
  syn keyword cConstant AG_MENU_INT_FLAGS AG_MENU_INT8_FLAGS AG_MENU_INT16_FLAGS
  syn keyword cConstant AG_MENU_INT32_FLAGS AG_MENU_DROPDOWN AG_MENU_POPUP
  syn keyword cConstant AG_MENU_GLOBAL AG_MENU_HFILL AG_MENU_VFILL
  syn keyword cConstant AG_MENU_NO_COLOR_BG AG_MENU_NO_BOOL_MSG
  syn keyword cConstant AG_MENU_FAST_BOOL_MSG AG_MENU_EXPAND
  " gui/mfspinbutton.h
  syn keyword cType AG_MFSpinbutton
  syn keyword cConstant AG_MFSPINBUTTON_UP AG_MFSPINBUTTON_LEFT
  syn keyword cConstant AG_MFSPINBUTTON_DOWN AG_MFSPINBUTTON_RIGHT
  syn keyword cConstant AG_MFSPINBUTTON_NOHFILL AG_MFSPINBUTTON_VFILL
  syn keyword cConstant AG_MFSPINBUTTON_EXCL
  " gui/mouse.h
  syn keyword cType AG_Mouse AG_MouseButton AG_MouseButton AG_MouseButtonAction
  syn keyword cConstant AG_MOUSE_NONE AG_MOUSE_LEFT AG_MOUSE_MIDDLE
  syn keyword cConstant AG_MOUSE_RIGHT AG_MOUSE_WHEELUP AG_MOUSE_WHEELDOWN
  syn keyword cConstant AG_MOUSE_X1 AG_MOUSE_X2 AG_MOUSE_ANY
  syn keyword cConstant AG_BUTTON_PRESSED AG_BUTTON_RELEASED
  syn keyword cConstant AG_MOUSE_LMASK AG_MOUSE_MMASK AG_MOUSE_RMASK
  " gui/mpane.h
  syn keyword cType AG_MPane
  syn keyword cConstant AG_MPANE1 AG_MPANE2V AG_MPANE2H AG_MPANE2L1R
  syn keyword cConstant AG_MPANE1L2R AG_MPANE2T1B AG_MPANE1T2B AG_MPANE3L1R
  syn keyword cConstant AG_MPANE1L3R AG_MPANE3T1B AG_MPANE1T3B AG_MPANE4
  syn keyword cConstant AG_MPANE_HFILL AG_MPANE_VFILL AG_MPANE_FRAMES
  syn keyword cConstant AG_MPANE_FORCE_DIV AG_MPANE_EXPAND
  " gui/mspinbutton.h
  syn keyword cType AG_MSpinbutton
  syn keyword cConstant AG_MSPINBUTTON_UP AG_MSPINBUTTON_LEFT
  syn keyword cConstant AG_MSPINBUTTON_DOWN AG_MSPINBUTTON_RIGHT
  syn keyword cConstant AG_MSPINBUTTON_HFILL AG_MSPINBUTTON_VFILL
  " gui/notebook.h
  syn keyword cType AG_Notebook AG_NotebookTab
  syn keyword cConstant AG_NOTEBOOK_LABEL_MAX AG_NOTEBOOK_HFILL
  syn keyword cConstant AG_NOTEBOOK_VFILL AG_NOTEBOOK_HIDE_TABS
  syn keyword cConstant AG_NOTEBOOK_EXPAND
  " gui/numerical.h
  syn keyword cType AG_Numerical
  syn keyword cConstant AG_NUMERICAL_INPUT_MAX AG_NUMERICAL_HFILL
  syn keyword cConstant AG_NUMERICAL_VFILL AG_NUMERICAL_INT AG_NUMERICAL_EXCL
  syn keyword cConstant AG_NUMERICAL_READONLY AG_NUMERICAL_SLOW
  syn keyword cConstant AG_NUMERICAL_EXPAND
  " gui/objsel.h
  syn keyword cType AG_ObjectSelector 
  syn keyword cConstant AG_OBJSEL_PAGE_DATA AG_OBJSEL_PAGE_GFX
  " gui/pane.h
  syn keyword cType AG_Pane
  syn keyword cConstant AG_PANE_HORIZ AG_PANE_VERT AG_PANE_TYPE_LAST
  syn keyword cConstant AG_PANE_EXPAND_DIV1 AG_PANE_EXPAND_DIV2
  syn keyword cConstant AG_PANE_DIVIDE_EVEN AG_PANE_DIVIDE_PCT
  syn keyword cConstant AG_PANE_HFILL AG_PANE_VFILL AG_PANE_DIV1FILL
  syn keyword cConstant AG_PANE_FRAME AG_PANE_UNMOVABLE AG_PANE_OVERRIDE_WDIV
  syn keyword cConstant AG_PANE_EXPAND
  " gui/pixmap.h
  syn keyword cConstant AG_PIXMAP_HFILL AG_PIXMAP_VFILL AG_PIXMAP_FORCE_SIZE
  syn keyword cConstant AG_PIXMAP_RESCALE AG_PIXMAP_UPDATE AG_PIXMAP_EXPAND
  " gui/primitive.h
  syn keyword cType AG_ArrowLineType AG_VectorElement AG_VectorElementType
  syn keyword cType AG_VectorElementStyle
  syn keyword cConstant AG_ARROWLINE_NONE AG_ARROWLINE_FORWARD
  syn keyword cConstant AG_ARROWLINE_REVERSE AG_ARROWLINE_BOTH
  syn keyword cConstant AG_VE_POINT AG_VE_LINE AG_VE_POLYGON AG_VE_CIRCLE
  syn keyword cConstant AG_VE_ARC1 AG_VE_ARC2 AG_VE_ARC3 AG_VE_ARC4 AG_VE_LAST
  syn keyword cConstant AG_VE_BEVELED AG_VE_ROUNDED AG_VE_FILLED AG_VE_TAG_MASK
  " gui/progress_bar.h
  syn keyword cType AG_ProgressBar
  syn keyword cConstant AG_PROGRESS_BAR_HORIZ AG_PROGRESS_BAR_VERT
  syn keyword cConstant AG_PROGRESS_BAR_HFILL AG_PROGRESS_BAR_VFILL
  syn keyword cConstant AG_PROGRESS_BAR_SHOW_PCT AG_PROGRESS_BAR_EXCL
  syn keyword cConstant AG_PROGRESS_BAR_EXPAND
  " gui/radio.h
  syn keyword cType AG_RadioType AG_RadioItem AG_Radio
  syn keyword cConstant AG_RADIO_TEXT_MAX AG_RADIO_VERT AG_RADIO_HORIZ
  syn keyword cConstant AG_RADIO_TYPE_LAST AG_RADIO_HFILL AG_RADIO_VFILL
  syn keyword cConstant AG_RADIO_HOMOGENOUS AG_RADIO_EXPAND
  " gui/scrollbar.h
  syn keyword cType AG_Scrollbar
  syn keyword cConstant AG_SCROLLBAR_HORIZ AG_SCROLLBAR_VERT
  syn keyword cConstant AG_SCROLLBAR_BUTTON_NONE AG_SCROLLBAR_BUTTON_DEC
  syn keyword cConstant AG_SCROLLBAR_BUTTON_INC AG_SCROLLBAR_BUTTON_SCROLL
  syn keyword cConstant AG_SCROLLBAR_HFILL AG_SCROLLBAR_VFILL AG_SCROLLBAR_SMOOTH
  syn keyword cConstant AG_SCROLLBAR_TEXT AG_SCROLLBAR_EXCL AG_SCROLLBAR_EXPAND
  " gui/scrollview.h
  syn keyword cType AG_Scrollview
  syn keyword cConstant AG_SCROLLVIEW_HORIZ AG_SCROLLVIEW_VERT
  syn keyword cConstant AG_SCROLLVIEW_STYLE_NONE AG_SCROLLVIEW_STYLE_BOX
  syn keyword cConstant AG_SCROLLVIEW_STYLE_WELL AG_SCROLLVIEW_STYLE_PLAIN
  syn keyword cConstant AG_SCROLLVIEW_STYLE_LAST
  syn keyword cConstant AG_SCROLLVIEW_HFILL AG_SCROLLVIEW_VFILL
  syn keyword cConstant AG_SCROLLVIEW_NOPAN_X AG_SCROLLVIEW_NOPAN_Y
  syn keyword cConstant AG_SCROLLVIEW_PANNING AG_SCROLLVIEW_BY_MOUSE
  syn keyword cConstant AG_SCROLLVIEW_FRAME AG_SCROLLVIEW_PAN_RIGHT
  syn keyword cConstant AG_SCROLLVIEW_PAN_LEFT AG_SCROLLVIEW_EXPAND
  syn keyword cConstant AG_SCROLLVIEW_NOPAN_XY
  " gui/separator.h
  syn keyword cType AG_Separator
  syn keyword cConstant AG_SEPARATOR_HORIZ AG_SEPARATOR_VERT
  " gui/slider.h
  syn keyword cType AG_Slider
  syn keyword cConstant AG_SLIDER_HORIZ AG_SLIDER_VERT
  syn keyword cConstant AG_SLIDER_BUTTON_NONE AG_SLIDER_BUTTON_DEC
  syn keyword cConstant AG_SLIDER_BUTTON_INC AG_SLIDER_BUTTON_SCROLL
  syn keyword cConstant AG_SLIDER_HFILL AG_SLIDER_VFILL AG_SLIDER_FOCUSABLE
  syn keyword cConstant AG_SLIDER_EXCL AG_SLIDER_EXPAND
  " gui/socket.h
  syn keyword cType AG_Socket
  syn keyword cConstant AG_SOCKET_PIXMAP AG_SOCKET_RECT AG_SOCKET_CIRCLE
  syn keyword cConstant AG_SOCKET_HFILL AG_SOCKET_VFILL AG_SOCKET_EXPAND
  syn keyword cConstant AG_SOCKET_MOUSEOVER AG_SOCKET_STICKY_STATE
  " gui/statusbar.h
  syn keyword cType AG_Statusbar
  syn keyword cConstant AG_STATUSBAR_MAX_LABELS AG_STATUSBAR_HFILL
  syn keyword cConstant AG_STATUSBAR_VFILL AG_STATUSBAR_EXPAND
  " gui/stylesheet.h
  syn keyword cType AG_StyleSheet AG_StyleEntry AG_StyleBlock AG_StaticCSS 
  syn keyword cConstant AG_STYLE_VALUE_MAX
  " gui/surface.h
  syn keyword cType AG_SurfaceMode AG_GrayscaleMode AG_Palette AG_PixelFormat 
  syn keyword cType AG_AnimDispose AG_AnimFrameType AG_AnimFrame AG_SurfaceGuide
  syn keyword cType AG_Surface AG_AnimState AG_TextureEnvMode AG_AlphaFn
  syn keyword cConstant AG_SURFACE_PACKED AG_SURFACE_INDEXED AG_SURFACE_GRAYSCALE
  syn keyword cConstant AG_GRAYSCALE_BT709 AG_GRAYSCALE_RMY AG_GRAYSCALE_Y
  syn keyword cConstant AG_DISPOSE_UNSPECIFIED AG_DISPOSE_DO_NOT
  syn keyword cConstant AG_DISPOSE_BACKGROUND AG_DISPOSE_PREVIOUS
  syn keyword cConstant AG_ANIM_FRAME_NONE AG_ANIM_FRAME_PIXELS
  syn keyword cConstant AG_ANIM_FRAME_COLORS AG_ANIM_FRAME_BLEND
  syn keyword cConstant AG_ANIM_FRAME_MOVE AG_ANIM_FRAME_DATA
  syn keyword cConstant AG_ANIM_FRAME_LAST AG_ANIM_FRAME_USER_INPUT
  syn keyword cConstant AG_SURFACE_GUIDE_TOP AG_SURFACE_GUIDE_RIGHT
  syn keyword cConstant AG_SURFACE_GUIDE_BOTTOM AG_SURFACE_GUIDE_LEFT
  syn keyword cConstant AG_SURFACE_NGUIDES AG_SURFACE_COLORKEY AG_SURFACE_ALPHA
  syn keyword cConstant AG_SURFACE_GL_TEXTURE AG_SURFACE_MAPPED AG_SURFACE_STATIC
  syn keyword cConstant AG_SURFACE_EXT_PIXELS AG_SURFACE_ANIMATED AG_SURFACE_TRACE
  syn keyword cConstant AG_SAVED_SURFACE_FLAGS AG_ANIM_LOOP AG_ANIM_PINGPONG
  syn keyword cConstant AG_ANIM_REVERSE AG_ANIM_PLAYING
  syn keyword cConstant AG_TEXTURE_ENV_MODULATE AG_TEXTURE_ENV_DECAL
  syn keyword cConstant AG_TEXTURE_ENV_BLEND AG_TEXTURE_ENV_REPLACE
  syn keyword cConstant AG_ALPHA_OVERLAY AG_ALPHA_ZERO AG_ALPHA_ONE AG_ALPHA_SRC
  syn keyword cConstant AG_ALPHA_DST AG_ALPHA_ONE_MINUS_DST AG_ALPHA_ONE_MINUS_SRC
  syn keyword cConstant AG_ALPHA_LAST AG_EXPORT_BMP_NO_32BIT AG_EXPORT_BMP_LEGACY
  syn keyword cConstant AG_EXPORT_PNG_ADAM7 AG_EXPORT_JPEG_JDCT_ISLOW
  syn keyword cConstant AG_EXPORT_JPEG_JDCT_IFAST AG_EXPORT_JPEG_JDCT_FLOAT
  " gui/table.h
  syn keyword cType AG_TablePopup AG_TableCell AG_TableBucket AG_TableCol
  syn keyword cType AG_Table
  syn keyword cConstant AG_TABLE_BUF_MAX AG_TABLE_COL_NAME_MAX
  syn keyword cConstant AG_TABLE_FMT_MAX AG_TABLE_HASHBUF_MAX
  syn keyword cConstant AG_TABLE_SEL_ROWS AG_TABLE_SEL_CELLS AG_TABLE_SEL_COLS
  syn keyword cConstant AG_TABLE_SEL_LAST
  syn keyword cConstant AG_TABLE_FN_POLL AG_TABLE_FN_ROW_CLICK
  syn keyword cConstant AG_TABLE_FN_ROW_DBLCLICK AG_TABLE_FN_COL_CLICK
  syn keyword cConstant AG_TABLE_FN_COL_DBLCLICK AG_TABLE_FN_CELL_CLICK
  syn keyword cConstant AG_TABLE_FN_CELL_DBLCLICK AG_TABLE_FN_LAST
  syn keyword cConstant AG_CELL_NULL AG_CELL_STRING AG_CELL_INT AG_CELL_UINT
  syn keyword cConstant AG_CELL_LONG AG_CELL_ULONG AG_CELL_FLOAT AG_CELL_DOUBLE
  syn keyword cConstant AG_CELL_PSTRING AG_CELL_PINT AG_CELL_PUINT AG_CELL_PLONG
  syn keyword cConstant AG_CELL_PULONG AG_CELL_PUINT8 AG_CELL_PSINT8
  syn keyword cConstant AG_CELL_PUINT16 AG_CELL_PSINT16 AG_CELL_PUINT32
  syn keyword cConstant AG_CELL_PSINT32 AG_CELL_PFLOAT AG_CELL_PDOUBLE
  syn keyword cConstant AG_CELL_SINT64 AG_CELL_UINT64 AG_CELL_PINT64
  syn keyword cConstant AG_CELL_PUINT64 AG_CELL_POINTER AG_CELL_FN_SU
  syn keyword cConstant AG_CELL_FN_SU_NODUP AG_CELL_FN_TXT AG_CELL_WIDGET
  syn keyword cConstant AG_TABLE_COL_FILL AG_TABLE_COL_ASCENDING
  syn keyword cConstant AG_TABLE_COL_DESCENDING AG_TABLE_COL_SELECTED
  syn keyword cConstant AG_TABLE_MULTI AG_TABLE_MULTITOGGLE AG_TABLE_REDRAW_CELLS
  syn keyword cConstant AG_TABLE_POLL AG_TABLE_HFILL AG_TABLE_VFILL AG_TABLE_EXPAND
  syn keyword cConstant AG_TABLE_HIGHLIGHT_COLS AG_TABLE_WIDGETS AG_TABLE_NOAUTOSORT
  syn keyword cConstant AG_TABLE_NEEDSORT AG_TABLE_COL_SELECT AG_TABLE_COL_SORT
  " gui/text.h
  syn keyword cType AG_TextANSI AG_FontSpec AG_FontAdjustment AG_Glyph AG_Font
  syn keyword cType AG_FontQ AG_TextState AG_StaticFont AG_TextMetrics
  syn keyword cType AG_GlyphCache AG_TTFFont AG_TTFGlyph
  syn keyword cConstant AG_TEXT_STATES_MAX AG_TEXT_FONTSPEC_MAX
  syn keyword cConstant AG_GLYPH_NBUCKETS AG_TEXT_ANSI_SEQ_MAX
  syn keyword cConstant AG_TEXT_ANSI_PARAM_MAX
  syn keyword cConstant AG_TEXT_LEFT AG_TEXT_CENTER AG_TEXT_RIGHT
  syn keyword cConstant AG_TEXT_TOP AG_TEXT_MIDDLE AG_TEXT_BOTTOM
  syn keyword cConstant AG_MSG_ERROR AG_MSG_WARNING AG_MSG_INFO
  syn keyword cConstant AG_FONT_VECTOR AG_FONT_BITMAP AG_FONT_DUMMY
  syn keyword cConstant AG_FONT_TYPE_LAST AG_FONT_SOURCE_FILE
  syn keyword cConstant AG_FONT_SOURCE_MEMORY AG_FONT_PTS_EPSILON
  syn keyword cConstant AG_ANSI_BLACK AG_ANSI_RED AG_ANSI_GREEN
  syn keyword cConstant AG_ANSI_YELLOW AG_ANSI_BLUE AG_ANSI_MAGENTA
  syn keyword cConstant AG_ANSI_CYAN AG_ANSI_WHITE AG_ANSI_BRIGHT_BLACK
  syn keyword cConstant AG_ANSI_BRIGHT_RED AG_ANSI_BRIGHT_GREEN
  syn keyword cConstant AG_ANSI_BRIGHT_YELLOW AG_ANSI_BRIGHT_BLUE
  syn keyword cConstant AG_ANSI_BRIGHT_MAGENTA AG_ANSI_BRIGHT_CYAN
  syn keyword cConstant AG_ANSI_BRIGHT_WHITE AG_ANSI_COLOR_LAST
  syn keyword cConstant AG_ANSI_SS2 AG_ANSI_SS3 AG_ANSI_DCS AG_ANSI_CSI_CUU
  syn keyword cConstant AG_ANSI_CSI_CUD AG_ANSI_CSI_CUF AG_ANSI_CSI_CUB
  syn keyword cConstant AG_ANSI_CSI_CNL AG_ANSI_CSI_CPL AG_ANSI_CSI_CHA
  syn keyword cConstant AG_ANSI_CSI_CUP AG_ANSI_CSI_ED AG_ANSI_CSI_EL
  syn keyword cConstant AG_ANSI_CSI_SU AG_ANSI_CSI_SD AG_ANSI_CSI_SGR
  syn keyword cConstant AG_ANSI_CSI_DSR AG_ANSI_CSI_SCP AG_ANSI_CSI_RCP
  syn keyword cConstant AG_ANSI_ST AG_ANSI_OSC AG_ANSI_PM AG_ANSI_APC
  syn keyword cConstant AG_ANSI_SOS AG_ANSI_RIS AG_ANSI_PRIVATE AG_ANSI_LAST
  syn keyword cConstant AG_SGR_RESET AG_SGR_BOLD AG_SGR_FAINT AG_SGR_ITALIC
  syn keyword cConstant AG_SGR_UNDERLINE AG_SGR_BLINK_SLOW AG_SGR_BLINK_FAST
  syn keyword cConstant AG_SGR_REVERSE AG_SGR_CONCEAL AG_SGR_CROSSED_OUT
  syn keyword cConstant AG_SGR_PRI_FONT AG_SGR_ALT_FONT_1 AG_SGR_ALT_FONT_2
  syn keyword cConstant AG_SGR_ALT_FONT_3 AG_SGR_ALT_FONT_4 AG_SGR_ALT_FONT_5
  syn keyword cConstant AG_SGR_ALT_FONT_6 AG_SGR_ALT_FONT_7 AG_SGR_ALT_FONT_8
  syn keyword cConstant AG_SGR_ALT_FONT_9 AG_SGR_FRAKTUR AG_SGR_UNDERLINE_2
  syn keyword cConstant AG_SGR_NO_INTENSITY AG_SGR_NO_FONT_STYLE AG_SGR_NO_UNDERLINE
  syn keyword cConstant AG_SGR_NO_BLINK AG_SGR_NO_INVERSE AG_SGR_REVEAL
  syn keyword cConstant AG_SGR_NOT_CROSSED_OUT AG_SGR_FG1 AG_SGR_FG2 AG_SGR_FG3
  syn keyword cConstant AG_SGR_FG4 AG_SGR_FG5 AG_SGR_FG6 AG_SGR_FG7 AG_SGR_FG8
  syn keyword cConstant AG_SGR_FG AG_SGR_NO_FG AG_SGR_BG1 AG_SGR_BG2 AG_SGR_BG3
  syn keyword cConstant AG_SGR_BG4 AG_SGR_BG5 AG_SGR_BG6 AG_SGR_BG7 AG_SGR_BG8
  syn keyword cConstant AG_SGR_BG AG_SGR_NO_BG AG_SGR_FRAMED AG_SGR_ENCIRCLED
  syn keyword cConstant AG_SGR_OVERLINED AG_SGR_NO_FRAMES AG_SGR_NOT_OVERLINED
  syn keyword cConstant AG_SGR_NO_FG_NO_BG AG_SGR_IDEOGRAM_1 AG_SGR_IDEOGRAM_2
  syn keyword cConstant AG_SGR_IDEOGRAM_3 AG_SGR_IDEOGRAM_4 AG_SGR_IDEOGRAM_5
  syn keyword cConstant AG_SGR_IDEOGRAM_6 AG_SGR_BRIGHT_FG_1 AG_SGR_BRIGHT_FG_2
  syn keyword cConstant AG_SGR_BRIGHT_FG_3 AG_SGR_BRIGHT_FG_4 AG_SGR_BRIGHT_FG_5
  syn keyword cConstant AG_SGR_BRIGHT_FG_6 AG_SGR_BRIGHT_FG_7 AG_SGR_BRIGHT_FG_8
  syn keyword cConstant AG_SGR_BRIGHT_BG_1 AG_SGR_BRIGHT_BG_2 AG_SGR_BRIGHT_BG_3
  syn keyword cConstant AG_SGR_BRIGHT_BG_4 AG_SGR_BRIGHT_BG_5 AG_SGR_BRIGHT_BG_6
  syn keyword cConstant AG_SGR_BRIGHT_BG_7 AG_SGR_BRIGHT_BG_8 AG_SGR_LAST
  syn keyword cConstant AG_FONT_BOLD AG_FONT_ITALIC AG_FONT_UNDERLINE
  syn keyword cConstant AG_FONT_UPPERCASE AG_FONT_OBLIQUE AG_FONT_UPRIGHT_ITALIC
  syn keyword cConstant AG_FONT_SEMICONDENSED AG_FONT_CONDENSED AG_FONT_SW_BOLD
  syn keyword cConstant AG_FONT_SW_OBLIQUE AG_FONT_MONOSPACE AG_FONT_SW_ITALIC
  syn keyword cConstant AG_FONT_WEIGHTS AG_FONT_STYLES AG_FONT_WD_VARIANTS
  " gui/text_cache.h
  syn keyword cType AG_TextCache AG_CachedText AG_TextCacheBucket
  " gui/textbox.h
  syn keyword cType AG_Textbox
  syn keyword cConstant AG_TEXTBOX_MULTILINE AG_TEXTBOX_UPPERCASE
  syn keyword cConstant AG_TEXTBOX_PASSWORD AG_TEXTBOX_ABANDON_FOCUS
  syn keyword cConstant AG_TEXTBOX_COMBO AG_TEXTBOX_HFILL AG_TEXTBOX_VFILL
  syn keyword cConstant AG_TEXTBOX_LOWERCASE AG_TEXTBOX_EXPAND AG_TEXTBOX_READONLY
  syn keyword cConstant AG_TEXTBOX_INT_ONLY AG_TEXTBOX_FLT_ONLY AG_TEXTBOX_CATCH_TAB
  syn keyword cConstant AG_TEXTBOX_CURSOR_MOVING AG_TEXTBOX_EXCL AG_TEXTBOX_NO_KILL_YANK
  syn keyword cConstant AG_TEXTBOX_RETURN_BUTTON AG_TEXTBOX_NO_ALT_LATIN1
  syn keyword cConstant AG_TEXTBOX_WORDWRAP AG_TEXTBOX_NOPOPUP AG_TEXTBOX_MULTILINGUAL
  syn keyword cConstant AG_TEXTBOX_NO_SHADING AG_TEXTBOX_UNDERSIZE
  " gui/titlebar.h
  syn keyword cType AG_Titlebar
  syn keyword cConstant AG_TITLEBAR_NO_CLOSE AG_TITLEBAR_NO_MINIMIZE
  syn keyword cConstant AG_TITLEBAR_NO_MAXIMIZE AG_TITLEBAR_PRESSED
  syn keyword cConstant AG_TITLEBAR_NO_BUTTONS AG_TITLEBAR_SAVED_FLAGS
  syn keyword cConstant AG_TLIST_LABEL_MAX AG_TLIST_ITEM_TAG AG_TLIST_ITEM_TAG_LEN
  syn keyword cConstant AG_TLIST_ITEM_EXPANDED AG_TLIST_HAS_CHILDREN
  syn keyword cConstant AG_TLIST_NO_SELECT AG_TLIST_NO_POPUP AG_TLIST_ITEM_DISABLED
  syn keyword cConstant AG_TLIST_MULTI AG_TLIST_MULTITOGGLE AG_TLIST_POLL
  syn keyword cConstant AG_TLIST_NO_SELECTED AG_TLIST_NO_SCALE_ICON
  syn keyword cConstant AG_TLIST_HFILL AG_TLIST_VFILL AG_TLIST_FIXED_HEIGHT
  syn keyword cConstant AG_TLIST_STATELESS AG_TLIST_SCROLLTOSEL AG_TLIST_REFRESH
  syn keyword cConstant AG_TLIST_EXPAND_NODES AG_TLIST_EXPAND
  " gui/tlist.h
  syn keyword cType AG_TlistPopup AG_TlistItem AG_TlistItemQ AG_Tlist 
  " gui/toolbar.h
  syn keyword cType AG_Toolbar 
  syn keyword cConstant AG_TOOLBAR_MAX_ROWS AG_TOOLBAR_HORIZ AG_TOOLBAR_VERT
  syn keyword cConstant AG_TOOLBAR_HOMOGENOUS AG_TOOLBAR_STICKY
  syn keyword cConstant AG_TOOLBAR_MULTI_STICKY AG_TOOLBAR_HFILL 
  syn keyword cConstant AG_TOOLBAR_VFILL AG_TOOLBAR_EXPAND
  " gui/treetbl.h
  syn keyword cType AG_TreetblDataFn AG_TreetblSortFn
  syn keyword cType AG_TreetblCol AG_TreetblCell AG_TreetblRow AG_TreetblRowQ
  syn keyword cType AG_Treetbl
  syn keyword cConstant AG_TREETBL_SORT_NOT AG_TREETBL_SORT_ASC
  syn keyword cConstant AG_TREETBL_SORT_DSC AG_TREETBL_COL_DYNAMIC
  syn keyword cConstant AG_TREETBL_COL_EXPANDER AG_TREETBL_COL_RESIZABLE
  syn keyword cConstant AG_TREETBL_COL_MOVING AG_TREETBL_COL_FILL
  syn keyword cConstant AG_TREETBL_COL_SELECTED AG_TREETBL_COL_SORTING
  syn keyword cConstant AG_TREETBL_ROW_EXPANDED AG_TREETBL_ROW_DYNAMIC
  syn keyword cConstant AG_TREETBL_ROW_SELECTED AG_TREETBL_MULTI
  syn keyword cConstant AG_TREETBL_MULTITOGGLE AG_TREETBL_REORDERCOLS
  syn keyword cConstant AG_TREETBL_NODUPCHECKS AG_TREETBL_SORT AG_TREETBL_POLLED
  syn keyword cConstant AG_TREETBL_HFILL AG_TREETBL_VFILL AG_TREETBL_EXPAND
  " gui/ucombo.h
  syn keyword cType AG_UCombo 
  syn keyword cConstant AG_UCOMBO_HFILL AG_UCOMBO_VFILL AG_UCOMBO_POLL
  syn keyword cConstant AG_UCOMBO_SCROLLTOSEL AG_UCOMBO_EXPAND
  " gui/units.h
  syn keyword cType AG_Unit
  " gui/widget.h
  syn keyword cType AG_SizeReq AG_SizeAlloc AG_WidgetClass AG_SizeSpec AG_FlagDescr
  syn keyword cType AG_ActionType AG_Action AG_ActionVec AG_ActionEventType
  syn keyword cType AG_ActionTie AG_RedrawTie AG_CursorArea AG_CursorAreaQ
  syn keyword cType AG_WidgetPalette AG_WidgetGL AG_WidgetPvt AG_Widget AG_WidgetVec
  syn keyword cConstant AG_ACTION_NAME_MAX AG_WIDGET_BAD_SPEC AG_WIDGET_PIXELS
  syn keyword cConstant AG_WIDGET_PERCENT AG_WIDGET_STRINGLEN AG_WIDGET_FILL
  syn keyword cConstant AG_ACTION_FN AG_ACTION_SET_INT AG_ACTION_TOGGLE_INT
  syn keyword cConstant AG_ACTION_SET_FLAG AG_ACTION_TOGGLE_FLAG
  syn keyword cConstant AG_ACTION_ON_BUTTONDOWN AG_ACTION_ON_BUTTONUP
  syn keyword cConstant AG_ACTION_ON_KEYDOWN AG_ACTION_ON_KEYUP
  syn keyword cConstant AG_ACTION_ON_KEYREPEAT AG_ACTION_ON_BUTTON
  syn keyword cConstant AG_REDRAW_ON_CHANGE AG_REDRAW_ON_TICK
  syn keyword cConstant AG_DEFAULT_STATE AG_DISABLED_STATE AG_FOCUSED_STATE
  syn keyword cConstant AG_HOVER_STATE AG_WIDGET_NSTATES
  syn keyword cConstant AG_FG_COLOR AG_BG_COLOR AG_TEXT_COLOR AG_LINE_COLOR
  syn keyword cConstant AG_HIGH_COLOR AG_LOW_COLOR AG_SELECTION_COLOR
  syn keyword cConstant FG_COLOR BG_COLOR TEXT_COLOR LINE_COLOR HIGH_COLOR
  syn keyword cConstant LOW_COLOR SELECTION_COLOR
  syn keyword cConstant AG_UNUSED_COLOR AG_WIDGET_NCOLORS
  syn keyword cConstant AG_WIDGET_FOCUSABLE AG_WIDGET_FOCUSED
  syn keyword cConstant AG_WIDGET_UNFOCUSED_MOTION AG_WIDGET_UNFOCUSED_BUTTONUP
  syn keyword cConstant AG_WIDGET_UNFOCUSED_BUTTONDOWN AG_WIDGET_VISIBLE
  syn keyword cConstant AG_WIDGET_HFILL AG_WIDGET_VFILL AG_WIDGET_USE_OPENGL
  syn keyword cConstant AG_WIDGET_HIDE AG_WIDGET_DISABLED AG_WIDGET_MOUSEOVER
  syn keyword cConstant AG_WIDGET_CATCH_TAB AG_WIDGET_GL_RESHAPE
  syn keyword cConstant AG_WIDGET_UNDERSIZE AG_WIDGET_DISABLE_ON_ATTACH
  syn keyword cConstant AG_WIDGET_UNFOCUSED_KEYDOWN AG_WIDGET_UNFOCUSED_KEYUP
  syn keyword cConstant AG_WIDGET_UPDATE_WINDOW AG_WIDGET_QUEUE_SURFACE_BACKUP
  syn keyword cConstant AG_WIDGET_USE_TEXT AG_WIDGET_USE_MOUSEOVER
  syn keyword cConstant AG_WIDGET_EXPAND AG_WIDGET_SURFACE_NODUP
  syn keyword cConstant AG_WIDGET_SURFACE_REGEN
  " gui/window.h
  syn keyword cType AG_WindowCloseAction AG_WindowFadeCtx AG_WindowPvt
  syn keyword cType AG_Window AG_WindowQ AG_WindowVec
  syn keyword cConstant AG_WINDOW_CAPTION_MAX
  syn keyword cConstant AG_WINDOW_UPPER_LEFT AG_WINDOW_UPPER_CENTER AG_WINDOW_UPPER_RIGHT
  syn keyword cConstant AG_WINDOW_MIDDLE_LEFT AG_WINDOW_CENTER AG_WINDOW_MIDDLE_RIGHT
  syn keyword cConstant AG_WINDOW_LOWER_LEFT AG_WINDOW_LOWER_CENTER AG_WINDOW_LOWER_RIGHT
  syn keyword cConstant AG_WINDOW_HIDE AG_WINDOW_DETACH AG_WINDOW_IGNORE
  syn keyword cConstant AG_WINDOW_WM_NORMAL AG_WINDOW_WM_DESKTOP AG_WINDOW_WM_DOCK
  syn keyword cConstant AG_WINDOW_WM_TOOLBAR AG_WINDOW_WM_MENU AG_WINDOW_WM_UTILITY
  syn keyword cConstant AG_WINDOW_WM_SPLASH AG_WINDOW_WM_DIALOG AG_WINDOW_WM_DROPDOWN_MENU
  syn keyword cConstant AG_WINDOW_WM_POPUP_MENU AG_WINDOW_WM_TOOLTIP
  syn keyword cConstant AG_WINDOW_WM_NOTIFICATION AG_WINDOW_WM_COMBO
  syn keyword cConstant AG_WINDOW_WM_DND
  syn keyword cConstant AG_WINDOW_MODAL AG_WINDOW_MAXIMIZED AG_WINDOW_MINIMIZED
  syn keyword cConstant AG_WINDOW_KEEPABOVE AG_WINDOW_KEEPBELOW AG_WINDOW_DENYFOCUS
  syn keyword cConstant AG_WINDOW_NOTITLE AG_WINDOW_NOBORDERS AG_WINDOW_NOHRESIZE
  syn keyword cConstant AG_WINDOW_NOVRESIZE AG_WINDOW_NOCLOSE AG_WINDOW_NOMINIMIZE
  syn keyword cConstant AG_WINDOW_NOMAXIMIZE AG_WINDOW_TILING AG_WINDOW_MINSIZEPCT
  syn keyword cConstant AG_WINDOW_NOBACKGROUND AG_WINDOW_MAIN AG_WINDOW_FOCUSONATTACH
  syn keyword cConstant AG_WINDOW_HMAXIMIZE AG_WINDOW_VMAXIMIZE AG_WINDOW_NOMOVE
  syn keyword cConstant AG_WINDOW_UPDATECAPTION AG_WINDOW_DETACHING
  syn keyword cConstant AG_WINDOW_INHERIT_ZOOM AG_WINDOW_NOCURSORCHG
  syn keyword cConstant AG_WINDOW_FADEIN AG_WINDOW_FADEOUT AG_WINDOW_USE_TEXT
  syn keyword cConstant AG_WINDOW_NORESIZE AG_WINDOW_NOBUTTONS AG_WINDOW_PLAIN
endif
