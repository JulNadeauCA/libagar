------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                          A G A R  . W I D G E T                          --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces; use Interfaces;
with Interfaces.C;
with Interfaces.C.Strings;
with System;
with Agar.Types; use Agar.Types;
with Agar.Object;
with Agar.Event;
with Agar.Surface;
with Agar.Input_Device;

--
-- Base class for all Agar GUI widgets.
--

package Agar.Widget is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;
  package OBJ renames Agar.Object;
  package EV renames Agar.Event;
  package SU renames Agar.Surface;
  package INDEV renames Agar.Input_Device;

  use type C.int;
  use type C.unsigned;
  use type C.C_float;
  
  type Window;
  type Window_Access is access all Window with Convention => C;
  subtype Window_not_null_Access is not null Window_Access;
  
  type Widget;
  type Widget_Access is access all Widget with Convention => C;
  subtype Widget_not_null_Access is not null Widget_Access;

  type Keyboard;
  type Keyboard_Access is access all Keyboard with Convention => C;
  subtype Keyboard_not_null_Access is not null Keyboard_Access;

  -----------------------------
  -- Widget Size Requisition --
  -----------------------------
  type SizeReq is limited record
    W : C.int;				-- Width (px)
    H : C.int;				-- Height (px)
  end record
    with Convention => C;

  type SizeReq_Access is access all SizeReq with Convention => C;
  subtype SizeReq_not_null_Access is not null SizeReq_Access;
  
  ----------------------------
  -- Widget Size Allocation --
  ----------------------------
  type SizeAlloc is limited record
    W,H : C.int;			-- Size (px)
    X,Y : C.int;			-- Position in parent
  end record
    with Convention => C;

  type SizeAlloc_Access is access all SizeAlloc with Convention => C;
  subtype SizeAlloc_not_null_Access is not null SizeAlloc_Access;

  -----------------------------------------------------------------------------
  --                              D R I V E R                                --
  -----------------------------------------------------------------------------
  
  type Driver_Type_t is
    (FRAME_BUFFER,                               -- By video memory access
     VECTOR);                                    -- By vector drawing commands
  for Driver_Type_t'Size use C.int'Size;

  type Driver_WM_Type_t is
    (WM_SINGLE,                                  -- Single-window (built-in WM)
     WM_MULTIPLE);                               -- Multi-windows
  for Driver_WM_Type_t'Size use C.int'Size;
  
  type Driver_Event;
  type Driver_Event_Access is access all Driver_Event with Convention => C;
  subtype Driver_Event_not_null_Access is not null Driver_Event_Access;

  ------------------
  -- Mouse Button --
  ------------------
  type Mouse_Button is
    (NONE,
     LEFT,
     MIDDLE,
     RIGHT,
     WHEEL_UP,
     WHEEL_DOWN,
     X1,
     X2,
     ANY);
  for Mouse_Button use
    (NONE       => 16#00_00#,
     LEFT       => 16#00_01#,
     MIDDLE     => 16#00_02#,
     RIGHT      => 16#00_03#,
     WHEEL_UP   => 16#00_04#,
     WHEEL_DOWN => 16#00_05#,
     X1         => 16#00_06#,
     X2         => 16#00_07#,
     ANY        => 16#00_ff#);
  for Mouse_Button'Size use C.unsigned'Size;
  
  -------------------------
  -- Mouse Button Action --
  -------------------------
  type Mouse_Button_Action is
    (PRESSED,
     RELEASED);
  for Mouse_Button_Action use
    (PRESSED  => 0,
     RELEASED => 1);
  for Mouse_Button_Action'Size use C.int'Size;

  ------------------
  -- Mouse Device --
  ------------------
  type Mouse is limited record
    Super        : aliased INDEV.Input_Device; -- ( Input_Device -> Mouse )
    Button_Count : C.unsigned;                 -- Button count (or 0)
    Button_State : Mouse_Button;               -- Last button state
    X,Y          : C.int;                      -- Last cursor position
    Xrel, Yrel   : C.int;                      -- Last relative motion
  end record
    with Convention => C;

  type Mouse_Access is access all Mouse with Convention => C;
  subtype Mouse_not_null_Access is not null Mouse_Access;

  ------------------
  -- Mouse Cursor --
  ------------------
  type Cursor;
  type Cursor_Access is access all Cursor with Convention => C;
  subtype Cursor_not_null_Access is not null Cursor_Access;

  type Cursor_Data_Access is access all Unsigned_8 with Convention => C;
  type Cursor_Entry is limited record
    Next : Cursor_Access;
    Prev : access Cursor_Access;
  end record
    with Convention => C;

  type Cursor_List is limited record
    First : Cursor_Access;
    Last  : access Cursor_Access;
  end record
    with Convention => C;

  type Cursor is limited record
    W,H             : C.unsigned;
    Bitmap          : Cursor_Data_Access;
    Mask            : Cursor_Data_Access;
    Hot_X, Hot_Y    : C.int;
    Driver_Data     : System.Address;
    Entry_in_Driver : Cursor_Entry;
  end record
    with Convention => C;

  ------------------------
  -- Categories of Keys --
  ------------------------
  type Key_Category is
    (KCAT_NONE,       -- No category
     KCAT_CONTROL,    -- Control character
     KCAT_SPACING,    -- Whitespace
     KCAT_RETURN,     -- Return / Line feed
     KCAT_PRINT,      -- Printable character (not alphanumeric)
     KCAT_ALPHA,      -- Alphabetic character
     KCAT_NUMBER,     -- Numerical character
     KCAT_DIR,        -- Directional key
     KCAT_FUNCTION,   -- Function key
     KCAT_LOCK,       -- (Num | Caps | Scroll) lock key
     KCAT_MODIFIER,   -- (Shift | Ctrl | Alt | Meta | Super | Compose) key
     KCAT_LAST);
  for Key_Category use
    (KCAT_NONE     => 0,
     KCAT_CONTROL  => 1,
     KCAT_SPACING  => 2,
     KCAT_RETURN   => 3,
     KCAT_PRINT    => 4,
     KCAT_ALPHA    => 5,
     KCAT_NUMBER   => 6,
     KCAT_DIR      => 7,
     KCAT_FUNCTION => 8,
     KCAT_LOCK     => 9,
     KCAT_MODIFIER => 10,
     KCAT_LAST     => 11);

  ---------------------
  -- Virtual Keysyms --
  ---------------------
  type Key_Sym is
    (KEY_NONE,
     KEY_BACKSPACE,
     KEY_TAB,
     KEY_CLEAR,
     KEY_RETURN,
     KEY_PAUSE,
     KEY_ESCAPE,
     KEY_SPACE,
     KEY_EXCLAIM,
     KEY_QUOTEDBL,
     KEY_HASH,
     KEY_DOLLAR,
     KEY_PERCENT,
     KEY_AMPERSAND,
     KEY_QUOTE,
     KEY_LEFT_PAREN,
     KEY_RIGHT_PAREN,
     KEY_ASTERISK,
     KEY_PLUS,
     KEY_COMMA,
     KEY_MINUS,
     KEY_PERIOD,
     KEY_SLASH,
     KEY_0,
     KEY_1,
     KEY_2,
     KEY_3,
     KEY_4,
     KEY_5,
     KEY_6,
     KEY_7,
     KEY_8,
     KEY_9,
     KEY_COLON,
     KEY_SEMICOLON,
     KEY_LESS,
     KEY_EQUALS,
     KEY_GREATER,
     KEY_QUESTION,
     KEY_AT,
     KEY_LEFT_BRACKET,
     KEY_BACKSLASH,
     KEY_RIGHT_BRACKET,
     KEY_CARET,
     KEY_UNDERSCORE,
     KEY_BACKQUOTE,
     KEY_A,
     KEY_B,
     KEY_C,
     KEY_D,
     KEY_E,
     KEY_F,
     KEY_G,
     KEY_H,
     KEY_I,
     KEY_J,
     KEY_K,
     KEY_L,
     KEY_M,
     KEY_N,
     KEY_O,
     KEY_P,
     KEY_Q,
     KEY_R,
     KEY_S,
     KEY_T,
     KEY_U,
     KEY_V,
     KEY_W,
     KEY_X,
     KEY_Y,
     KEY_Z,
     KEY_DELETE,
     KEY_KP0,
     KEY_KP1,
     KEY_KP2,
     KEY_KP3,
     KEY_KP4,
     KEY_KP5,
     KEY_KP6,
     KEY_KP7,
     KEY_KP8,
     KEY_KP9,
     KEY_KP_PERIOD,
     KEY_KP_DIVIDE,
     KEY_KP_MULTIPLY,
     KEY_KP_MINUS,
     KEY_KP_PLUS,
     KEY_KP_ENTER,
     KEY_KP_EQUALS,
     KEY_UP,
     KEY_DOWN,
     KEY_RIGHT,
     KEY_LEFT,
     KEY_INSERT,
     KEY_HOME,
     KEY_END,
     KEY_PAGE_UP,
     KEY_PAGE_DOWN,
     KEY_F1,
     KEY_F2,
     KEY_F3,
     KEY_F4,
     KEY_F5,
     KEY_F6,
     KEY_F7,
     KEY_F8,
     KEY_F9,
     KEY_F10,
     KEY_F11,
     KEY_F12,
     KEY_F13,
     KEY_F14,
     KEY_F15,
     KEY_NUM_LOCK,
     KEY_CAPS_LOCK,
     KEY_SCROLL_LOCK,
     KEY_RIGHT_SHIFT,
     KEY_LEFT_SHIFT,
     KEY_RIGHT_CTRL,
     KEY_LEFT_CTRL,
     KEY_RIGHT_ALT,
     KEY_LEFT_ALT,
     KEY_RIGHT_META,
     KEY_LEFT_META,
     KEY_LEFT_SUPER,
     KEY_RIGHT_SUPER,
     KEY_MODE,
     KEY_COMPOSE,
     KEY_HELP,
     KEY_PRINT,
     KEY_SYSREQ,
     KEY_BREAK,
     KEY_MENU,
     KEY_POWER,
     KEY_EURO,
     KEY_UNDO,
     KEY_GRAVE,
     KEY_KP_CLEAR,
     KEY_COMMAND,
     KEY_FUNCTION,
     KEY_VOLUME_UP,
     KEY_VOLUME_DOWN,
     KEY_VOLUME_MUTE,
     KEY_F16,
     KEY_F17,
     KEY_F18,
     KEY_F19,
     KEY_F20,
     KEY_F21,
     KEY_F22,
     KEY_F23,
     KEY_F24,
     KEY_F25,
     KEY_F26,
     KEY_F27,
     KEY_F28,
     KEY_F29,
     KEY_F30,
     KEY_F31,
     KEY_F32,
     KEY_F33,
     KEY_F34,
     KEY_F35,
     KEY_BEGIN,
     KEY_RESET,
     KEY_STOP,
     KEY_USER,
     KEY_SYSTEM,
     KEY_PRINT_SCREEN,
     KEY_CLEAR_LINE,
     KEY_CLEAR_DISPLAY,
     KEY_INSERT_LINE,
     KEY_DELETE_LINE,
     KEY_INSERT_CHAR,
     KEY_DELETE_CHAR,
     KEY_PREV,
     KEY_NEXT,
     KEY_SELECT,
     KEY_EXECUTE,
     KEY_REDO,
     KEY_FIND,
     KEY_MODE_SWITCH,
     KEY_NON_US_BACKSLASH,
     KEY_APPLICATION,
     KEY_AGAIN,
     KEY_CUT,
     KEY_PASTE,
     KEY_KP_COMMA,
     KEY_KP_EQUALS_AS_400,
     KEY_INTERNATIONAL_1,
     KEY_INTERNATIONAL_2,
     KEY_INTERNATIONAL_3,
     KEY_INTERNATIONAL_4,
     KEY_INTERNATIONAL_5,
     KEY_INTERNATIONAL_6,
     KEY_INTERNATIONAL_7,
     KEY_INTERNATIONAL_8,
     KEY_INTERNATIONAL_9,
     KEY_LANGUAGE_1,
     KEY_LANGUAGE_2,
     KEY_LANGUAGE_3,
     KEY_LANGUAGE_4,
     KEY_LANGUAGE_5,
     KEY_LANGUAGE_6,
     KEY_LANGUAGE_7,
     KEY_LANGUAGE_8,
     KEY_LANGUAGE_9,
     KEY_ALT_ERASE,
     KEY_CANCEL,
     KEY_PRIOR,
     KEY_RETURN2,
     KEY_SEPARATOR,
     KEY_OUT,
     KEY_OPER,
     KEY_CLEAR_AGAIN,
     KEY_CRSEL,
     KEY_EXSEL,
     KEY_KP_00,
     KEY_KP_000,
     KEY_THOUSANDS_SEPARATOR,
     KEY_DECIMALS_SEPARATOR,
     KEY_CURRENCY_UNIT,
     KEY_CURRENCY_SUBUNIT,
     KEY_KP_LEFT_PAREN,
     KEY_KP_RIGHT_PAREN,
     KEY_KP_LEFT_BRACE,
     KEY_KP_RIGHT_BRACE,
     KEY_KP_TAB,
     KEY_KP_BACKSPACE,
     KEY_KP_A,
     KEY_KP_B,
     KEY_KP_C,
     KEY_KP_D,
     KEY_KP_E,
     KEY_KP_F,
     KEY_KP_XOR,
     KEY_KP_POWER,
     KEY_KP_PERCENT,
     KEY_KP_LESS,
     KEY_KP_GREATER,
     KEY_KP_AMPERSAND,
     KEY_KP_DBL_AMPERSAND,
     KEY_KP_VERTICAL_BAR,
     KEY_KP_DBL_VERTICAL_BAR,
     KEY_KP_COLON,
     KEY_KP_HASH,
     KEY_KP_SPACE,
     KEY_KP_AT,
     KEY_KP_EXCLAM,
     KEY_KP_MEM_STORE,
     KEY_KP_MEM_RECALL,
     KEY_KP_MEM_CLEAR,
     KEY_KP_MEM_ADD,
     KEY_KP_MEM_SUBTRACT,
     KEY_KP_MEM_MULTIPLY,
     KEY_KP_MEM_DIVIDE,
     KEY_KP_PLUS_MINUS,
     KEY_KP_CLEAR_ENTRY,
     KEY_KP_BINARY,
     KEY_KP_OCTAL,
     KEY_KP_DECIMAL,
     KEY_KP_HEXADECIMAL,
     KEY_LGUI,
     KEY_RGUI,
     KEY_AUDIO_NEXT,
     KEY_AUDIO_PREV,
     KEY_AUDIO_STOP,
     KEY_AUDIO_PLAY,
     KEY_AUDIO_MUTE,
     KEY_AUDIO_MEDIA_SELECT,
     KEY_WWW,
     KEY_MAIL,
     KEY_CALCULATOR,
     KEY_COMPUTER,
     KEY_AC_SEARCH,
     KEY_AC_HOME,
     KEY_AC_BACK,
     KEY_AC_FORWARD,
     KEY_AC_STOP,
     KEY_AC_REFRESH,
     KEY_AC_BOOKMARKS,
     KEY_AUDIO_REWIND,
     KEY_AUDIO_FASTFORWARD,
     KEY_LAST,
     KEY_ANY);

  for Key_Sym use
    (KEY_NONE                 => 16#00_00#,
     KEY_BACKSPACE            => 16#00_08#,
     KEY_TAB                  => 16#00_09#,
     KEY_CLEAR                => 16#00_0c#,
     KEY_RETURN               => 16#00_0d#,
     KEY_PAUSE                => 16#00_13#,
     KEY_ESCAPE               => 16#00_1b#,
     KEY_SPACE                => 16#00_20#,     --   --
     KEY_EXCLAIM              => 16#00_21#,	-- ! --
     KEY_QUOTEDBL             => 16#00_22#,	-- " --
     KEY_HASH                 => 16#00_23#,	-- # --
     KEY_DOLLAR               => 16#00_24#,	-- $ --
     KEY_PERCENT              => 16#00_25#,	-- % --
     KEY_AMPERSAND            => 16#00_26#,	-- & --
     KEY_QUOTE                => 16#00_27#,	-- ' --
     KEY_LEFT_PAREN           => 16#00_28#,	-- ( --
     KEY_RIGHT_PAREN          => 16#00_29#,	-- ) --
     KEY_ASTERISK             => 16#00_2a#,	-- * --
     KEY_PLUS                 => 16#00_2b#,	-- + --
     KEY_COMMA                => 16#00_2c#,	-- , --
     KEY_MINUS                => 16#00_2d#,	-- - --
     KEY_PERIOD               => 16#00_2e#,	-- . --
     KEY_SLASH                => 16#00_2f#,	-- / --
     KEY_0                    => 16#00_30#,	-- 0 --
     KEY_1                    => 16#00_31#,	-- 1 --
     KEY_2                    => 16#00_32#,	-- 2 --
     KEY_3                    => 16#00_33#,	-- 3 --
     KEY_4                    => 16#00_34#,	-- 4 --
     KEY_5                    => 16#00_35#,	-- 5 --
     KEY_6                    => 16#00_36#,	-- 6 --
     KEY_7                    => 16#00_37#,	-- 7 --
     KEY_8                    => 16#00_38#,	-- 8 --
     KEY_9                    => 16#00_39#,	-- 9 --
     KEY_COLON                => 16#00_3a#,	-- : --
     KEY_SEMICOLON            => 16#00_3b#,	-- ; --
     KEY_LESS                 => 16#00_3c#,	-- < --
     KEY_EQUALS               => 16#00_3d#,	-- = --
     KEY_GREATER              => 16#00_3e#,	-- > --
     KEY_QUESTION             => 16#00_3f#,	-- ? --
     KEY_AT                   => 16#00_40#,	-- @ --
     KEY_LEFT_BRACKET         => 16#00_5b#,	-- [ --
     KEY_BACKSLASH            => 16#00_5c#,	-- \ --
     KEY_RIGHT_BRACKET        => 16#00_5d#,	-- ] --
     KEY_CARET                => 16#00_5e#,	-- ^ --
     KEY_UNDERSCORE           => 16#00_5f#,	-- _ --
     KEY_BACKQUOTE            => 16#00_60#,	-- ` --
     KEY_A                    => 16#00_61#,	-- a --
     KEY_B                    => 16#00_62#,	-- b --
     KEY_C                    => 16#00_63#,	-- c --
     KEY_D                    => 16#00_64#,	-- d --
     KEY_E                    => 16#00_65#,	-- e --
     KEY_F                    => 16#00_66#,	-- f --
     KEY_G                    => 16#00_67#,	-- g --
     KEY_H                    => 16#00_68#,	-- h --
     KEY_I                    => 16#00_69#,	-- i --
     KEY_J                    => 16#00_6a#,	-- j --
     KEY_K                    => 16#00_6b#,	-- k --
     KEY_L                    => 16#00_6c#,	-- l --
     KEY_M                    => 16#00_6d#,	-- m --
     KEY_N                    => 16#00_6e#,	-- n --
     KEY_O                    => 16#00_6f#,	-- o --
     KEY_P                    => 16#00_70#,	-- p --
     KEY_Q                    => 16#00_71#,	-- q --
     KEY_R                    => 16#00_72#,	-- r --
     KEY_S                    => 16#00_73#,	-- s --
     KEY_T                    => 16#00_74#,	-- t --
     KEY_U                    => 16#00_75#,	-- u --
     KEY_V                    => 16#00_76#,	-- v --
     KEY_W                    => 16#00_77#,	-- w --
     KEY_X                    => 16#00_78#,	-- x --
     KEY_Y                    => 16#00_79#,	-- y --
     KEY_Z                    => 16#00_7a#,	-- z --
     KEY_DELETE               => 16#00_7f#,
     KEY_KP0                  => 16#01_00#,
     KEY_KP1                  => 16#01_01#,
     KEY_KP2                  => 16#01_02#,
     KEY_KP3                  => 16#01_03#,
     KEY_KP4                  => 16#01_04#,
     KEY_KP5                  => 16#01_05#,
     KEY_KP6                  => 16#01_06#,
     KEY_KP7                  => 16#01_07#,
     KEY_KP8                  => 16#01_08#,
     KEY_KP9                  => 16#01_09#,
     KEY_KP_PERIOD            => 16#01_0a#,
     KEY_KP_DIVIDE            => 16#01_0b#,
     KEY_KP_MULTIPLY          => 16#01_0c#,
     KEY_KP_MINUS             => 16#01_0d#,
     KEY_KP_PLUS              => 16#01_0e#,
     KEY_KP_ENTER             => 16#01_0f#,
     KEY_KP_EQUALS            => 16#01_10#,
     KEY_UP                   => 16#01_11#,
     KEY_DOWN                 => 16#01_12#,
     KEY_RIGHT                => 16#01_13#,
     KEY_LEFT                 => 16#01_14#,
     KEY_INSERT               => 16#01_15#,
     KEY_HOME                 => 16#01_16#,
     KEY_END                  => 16#01_17#,
     KEY_PAGE_UP              => 16#01_18#,
     KEY_PAGE_DOWN            => 16#01_19#,
     KEY_F1                   => 16#01_1a#,
     KEY_F2                   => 16#01_1b#,
     KEY_F3                   => 16#01_1c#,
     KEY_F4                   => 16#01_1d#,
     KEY_F5                   => 16#01_1e#,
     KEY_F6                   => 16#01_1f#,
     KEY_F7                   => 16#01_20#,
     KEY_F8                   => 16#01_21#,
     KEY_F9                   => 16#01_22#,
     KEY_F10                  => 16#01_23#,
     KEY_F11                  => 16#01_24#,
     KEY_F12                  => 16#01_25#,
     KEY_F13                  => 16#01_26#,
     KEY_F14                  => 16#01_27#,
     KEY_F15                  => 16#01_28#,
     KEY_NUM_LOCK             => 16#01_2c#,
     KEY_CAPS_LOCK            => 16#01_2d#,
     KEY_SCROLL_LOCK          => 16#01_2e#,
     KEY_RIGHT_SHIFT          => 16#01_2f#,
     KEY_LEFT_SHIFT           => 16#01_30#,
     KEY_RIGHT_CTRL           => 16#01_31#,
     KEY_LEFT_CTRL            => 16#01_32#,
     KEY_RIGHT_ALT            => 16#01_33#,
     KEY_LEFT_ALT             => 16#01_34#,
     KEY_RIGHT_META           => 16#01_35#,
     KEY_LEFT_META            => 16#01_36#,
     KEY_LEFT_SUPER           => 16#01_37#,
     KEY_RIGHT_SUPER          => 16#01_38#,
     KEY_MODE                 => 16#01_39#,
     KEY_COMPOSE              => 16#01_3a#,
     KEY_HELP                 => 16#01_3b#,
     KEY_PRINT                => 16#01_3c#,
     KEY_SYSREQ               => 16#01_3d#,
     KEY_BREAK                => 16#01_3e#,
     KEY_MENU                 => 16#01_3f#,
     KEY_POWER                => 16#01_40#,
     KEY_EURO                 => 16#01_41#,
     KEY_UNDO                 => 16#01_42#,
     KEY_GRAVE                => 16#01_43#,
     KEY_KP_CLEAR             => 16#01_44#,
     KEY_COMMAND              => 16#01_45#,
     KEY_FUNCTION             => 16#01_46#,
     KEY_VOLUME_UP            => 16#01_47#,
     KEY_VOLUME_DOWN          => 16#01_48#,
     KEY_VOLUME_MUTE          => 16#01_49#,
     KEY_F16                  => 16#01_4a#,
     KEY_F17                  => 16#01_4b#,
     KEY_F18                  => 16#01_4c#,
     KEY_F19                  => 16#01_4d#,
     KEY_F20                  => 16#01_4e#,
     KEY_F21                  => 16#01_4f#,
     KEY_F22                  => 16#01_50#,
     KEY_F23                  => 16#01_51#,
     KEY_F24                  => 16#01_52#,
     KEY_F25                  => 16#01_53#,
     KEY_F26                  => 16#01_54#,
     KEY_F27                  => 16#01_55#,
     KEY_F28                  => 16#01_56#,
     KEY_F29                  => 16#01_57#,
     KEY_F30                  => 16#01_58#,
     KEY_F31                  => 16#01_59#,
     KEY_F32                  => 16#01_5a#,
     KEY_F33                  => 16#01_5b#,
     KEY_F34                  => 16#01_5c#,
     KEY_F35                  => 16#01_5d#,
     KEY_BEGIN                => 16#01_5e#,
     KEY_RESET                => 16#01_5f#,
     KEY_STOP                 => 16#01_60#,
     KEY_USER                 => 16#01_61#,
     KEY_SYSTEM               => 16#01_62#,
     KEY_PRINT_SCREEN         => 16#01_63#,
     KEY_CLEAR_LINE           => 16#01_64#,
     KEY_CLEAR_DISPLAY        => 16#01_65#,
     KEY_INSERT_LINE          => 16#01_66#,
     KEY_DELETE_LINE          => 16#01_67#,
     KEY_INSERT_CHAR          => 16#01_68#,
     KEY_DELETE_CHAR          => 16#01_69#,
     KEY_PREV                 => 16#01_6a#,
     KEY_NEXT                 => 16#01_6b#,
     KEY_SELECT               => 16#01_6c#,
     KEY_EXECUTE              => 16#01_6d#,
     KEY_REDO                 => 16#01_6e#,
     KEY_FIND                 => 16#01_6f#,
     KEY_MODE_SWITCH          => 16#01_70#,
     KEY_NON_US_BACKSLASH     => 16#01_71#,
     KEY_APPLICATION          => 16#01_72#,
     KEY_AGAIN                => 16#01_73#,
     KEY_CUT                  => 16#01_74#,
     KEY_PASTE                => 16#01_75#,
     KEY_KP_COMMA             => 16#01_76#,
     KEY_KP_EQUALS_AS_400     => 16#01_77#,
     KEY_INTERNATIONAL_1      => 16#01_78#,
     KEY_INTERNATIONAL_2      => 16#01_79#,
     KEY_INTERNATIONAL_3      => 16#01_7a#,
     KEY_INTERNATIONAL_4      => 16#01_7b#,
     KEY_INTERNATIONAL_5      => 16#01_7c#,
     KEY_INTERNATIONAL_6      => 16#01_7d#,
     KEY_INTERNATIONAL_7      => 16#01_7e#,
     KEY_INTERNATIONAL_8      => 16#01_7f#,
     KEY_INTERNATIONAL_9      => 16#01_80#,
     KEY_LANGUAGE_1           => 16#01_81#,
     KEY_LANGUAGE_2           => 16#01_82#,
     KEY_LANGUAGE_3           => 16#01_83#,
     KEY_LANGUAGE_4           => 16#01_84#,
     KEY_LANGUAGE_5           => 16#01_85#,
     KEY_LANGUAGE_6           => 16#01_86#,
     KEY_LANGUAGE_7           => 16#01_87#,
     KEY_LANGUAGE_8           => 16#01_88#,
     KEY_LANGUAGE_9           => 16#01_89#,
     KEY_ALT_ERASE            => 16#01_90#,
     KEY_CANCEL               => 16#01_91#,
     KEY_PRIOR                => 16#01_92#,
     KEY_RETURN2              => 16#01_93#,
     KEY_SEPARATOR            => 16#01_94#,
     KEY_OUT                  => 16#01_95#,
     KEY_OPER                 => 16#01_96#,
     KEY_CLEAR_AGAIN          => 16#01_97#,
     KEY_CRSEL                => 16#01_98#,
     KEY_EXSEL                => 16#01_99#,
     KEY_KP_00                => 16#01_9a#,
     KEY_KP_000               => 16#01_9b#,
     KEY_THOUSANDS_SEPARATOR  => 16#01_9c#,
     KEY_DECIMALS_SEPARATOR   => 16#01_9d#,
     KEY_CURRENCY_UNIT        => 16#01_9e#,
     KEY_CURRENCY_SUBUNIT     => 16#01_9f#,
     KEY_KP_LEFT_PAREN        => 16#01_a0#,
     KEY_KP_RIGHT_PAREN       => 16#01_a1#,
     KEY_KP_LEFT_BRACE        => 16#01_a2#,
     KEY_KP_RIGHT_BRACE       => 16#01_a3#,
     KEY_KP_TAB               => 16#01_a4#,
     KEY_KP_BACKSPACE         => 16#01_a5#,
     KEY_KP_A                 => 16#01_a6#,
     KEY_KP_B                 => 16#01_a7#,
     KEY_KP_C                 => 16#01_a8#,
     KEY_KP_D                 => 16#01_a9#,
     KEY_KP_E                 => 16#01_aa#,
     KEY_KP_F                 => 16#01_ab#,
     KEY_KP_XOR               => 16#01_ac#,
     KEY_KP_POWER             => 16#01_ad#,
     KEY_KP_PERCENT           => 16#01_ae#,
     KEY_KP_LESS              => 16#01_af#,
     KEY_KP_GREATER           => 16#01_b1#,
     KEY_KP_AMPERSAND         => 16#01_b2#,
     KEY_KP_DBL_AMPERSAND     => 16#01_b3#,
     KEY_KP_VERTICAL_BAR      => 16#01_b4#,
     KEY_KP_DBL_VERTICAL_BAR  => 16#01_b5#,
     KEY_KP_COLON             => 16#01_b6#,
     KEY_KP_HASH              => 16#01_b7#,
     KEY_KP_SPACE             => 16#01_b8#,
     KEY_KP_AT                => 16#01_b9#,
     KEY_KP_EXCLAM            => 16#01_ba#,
     KEY_KP_MEM_STORE         => 16#01_bb#,
     KEY_KP_MEM_RECALL        => 16#01_bc#,
     KEY_KP_MEM_CLEAR         => 16#01_bd#,
     KEY_KP_MEM_ADD           => 16#01_be#,
     KEY_KP_MEM_SUBTRACT      => 16#01_bf#,
     KEY_KP_MEM_MULTIPLY      => 16#01_c0#,
     KEY_KP_MEM_DIVIDE        => 16#01_c1#,
     KEY_KP_PLUS_MINUS        => 16#01_c2#,
     KEY_KP_CLEAR_ENTRY       => 16#01_c3#,
     KEY_KP_BINARY            => 16#01_c4#,
     KEY_KP_OCTAL             => 16#01_c5#,
     KEY_KP_DECIMAL           => 16#01_c6#,
     KEY_KP_HEXADECIMAL       => 16#01_c7#,
     KEY_LGUI                 => 16#01_c8#,
     KEY_RGUI                 => 16#01_c9#,
     KEY_AUDIO_NEXT           => 16#01_ca#,
     KEY_AUDIO_PREV           => 16#01_cb#,
     KEY_AUDIO_STOP           => 16#01_cc#,
     KEY_AUDIO_PLAY           => 16#01_cd#,
     KEY_AUDIO_MUTE           => 16#01_ce#,
     KEY_AUDIO_MEDIA_SELECT   => 16#01_cf#,
     KEY_WWW                  => 16#01_d0#,
     KEY_MAIL                 => 16#01_d1#,
     KEY_CALCULATOR           => 16#01_d2#,
     KEY_COMPUTER             => 16#01_d3#,
     KEY_AC_SEARCH            => 16#01_d4#,
     KEY_AC_HOME              => 16#01_d5#,
     KEY_AC_BACK              => 16#01_d6#,
     KEY_AC_FORWARD           => 16#01_d7#,
     KEY_AC_STOP              => 16#01_d8#,
     KEY_AC_REFRESH           => 16#01_d9#,
     KEY_AC_BOOKMARKS         => 16#01_da#,
     KEY_AUDIO_REWIND         => 16#01_db#,
     KEY_AUDIO_FASTFORWARD    => 16#01_dc#,

     KEY_LAST                 => 16#01_de#,
     KEY_ANY                  => 16#ff_ff#);
  for Key_Sym'Size use C.int'Size;
  
  -------------------
  -- Key Modifiers --
  -------------------
  KEYMOD_NONE        : constant C.unsigned := 16#00_00#;
  KEYMOD_LEFT_SHIFT  : constant C.unsigned := 16#00_01#;
  KEYMOD_RIGHT_SHIFT : constant C.unsigned := 16#00_02#;
  KEYMOD_LEFT_CTRL   : constant C.unsigned := 16#00_40#;
  KEYMOD_RIGHT_CTRL  : constant C.unsigned := 16#00_80#;
  KEYMOD_LEFT_ALT    : constant C.unsigned := 16#01_00#;
  KEYMOD_RIGHT_ALT   : constant C.unsigned := 16#02_00#;
  KEYMOD_LEFT_META   : constant C.unsigned := 16#04_00#;
  KEYMOD_RIGHT_META  : constant C.unsigned := 16#08_00#;
  KEYMOD_NUM_LOCK    : constant C.unsigned := 16#10_00#;
  KEYMOD_CAPS_LOCK   : constant C.unsigned := 16#20_00#;
  KEYMOD_MODE        : constant C.unsigned := 16#40_00#;
  KEYMOD_ANY         : constant C.unsigned := 16#ff_ff#;

  KEYMOD_CTRL  : constant C.unsigned := KEYMOD_LEFT_CTRL  or KEYMOD_RIGHT_CTRL;
  KEYMOD_SHIFT : constant C.unsigned := KEYMOD_LEFT_SHIFT or KEYMOD_RIGHT_SHIFT;
  KEYMOD_ALT   : constant C.unsigned := KEYMOD_LEFT_ALT   or KEYMOD_RIGHT_ALT;
  KEYMOD_META  : constant C.unsigned := KEYMOD_LEFT_META  or KEYMOD_RIGHT_META;
 
  type Virtual_Key_t is record
    Symbol   : Key_Sym;                   -- Virtual key
    Modifier : C.int;                     -- Virtual key modifier mask
    Unicode  : AG_Char;                   -- Corresponding Unicode (or 0)
  end record
    with Convention => C;
  
  -----------------------------------------------------
  -- Widget Color Palette (4 states x 8 = 32 colors) --
  -----------------------------------------------------
  type Widget_State is
    (DEFAULT_STATE,			-- Not focused (default state)
     DISABLED_STATE,			-- Disabled (#disabled)
     FOCUSED_STATE,			-- Holds focus (#focused)
     HOVER_STATE);                      -- Cursor is over (#hover)
  for Widget_State'Size use C.int'Size;

  type Widget_Color is
    (FG_COLOR,            -- Foreground primary      ("color")
     BG_COLOR,            -- Background primary      ("background-color")
     TEXT_COLOR,          -- Text and vector icons   ("text-color")
     LINE_COLOR,          -- Lines and filled shapes ("line-color")
     HIGH_COLOR,          -- Top and left shading    ("high-color")
     LOW_COLOR,           -- Bottom/right shading    ("low-color")
     SELECTION_COLOR,     -- Selection primary       ("selection-color")
     UNUSED_COLOR);       -- Currently unused

  -- TODO
  type Widget_Palette is array (1 .. $SIZEOF_AG_WidgetPalette)
    of aliased Unsigned_8 with Convention => C;
  for Widget_Palette'Size use $SIZEOF_AG_WidgetPalette * System.Storage_Unit;
 
  -----------------------------
  -- GL & Texture Management --
  -----------------------------
#if HAVE_OPENGL
  type Widget_GL_Projection_Matrix is array (1 .. 16) of aliased C.C_float with Convention => C;
  type Widget_GL_Modelview_Matrix is array (1 .. 16) of aliased C.C_float with Convention => C;
  type Widget_GL_Context is limited record
    Projection : Widget_GL_Projection_Matrix;
    Modelview  : Widget_GL_Modelview_Matrix;
  end record
    with Convention => C;
  type Widget_GL_Context_Access is access all Widget_GL_Context with Convention => C;
#end if;
  subtype Surface_Handle is C.int;
  subtype Texture_Handle is C.unsigned;
  type Texture_Handle_Access is access all Texture_Handle with Convention => C;
  subtype Texture_Handle_not_null_Access is not null Texture_Handle_Access;

  type Texture_Coord is limited record
    X,Y : C.C_float;
    W,H : C.C_float;
  end record
    with Convention => C;

  type Texture_Coord_Access is access all Texture_Coord with Convention => C;
  subtype Texture_Coord_not_null_Access is not null Texture_Coord_Access;
  
  type Driver is limited record
    Super         : aliased OBJ.Object;          -- ( Object -> Driver )
    Instance_ID   : C.unsigned;                  -- Driver instance ID
    Flags         : C.unsigned;                  -- Flags (below)
    Ref_Surface   : SU.Surface_Access;           -- Standard surface format
    Video_Format  : SU.Pixel_Format;             -- Video format (FB modes only)
    Keyboard      : Keyboard_Access;             -- Keyboard device
    Mouse         : Mouse_Access;                -- Mouse device
    Glyph_Cache   : System.Address;              -- Cached AG_Glyph store
    GL_Context    : System.Address;              -- TODO: AG_GL_Context
    Active_Cursor : Cursor_Access;               -- Effective cursor
    Cursors       : Cursor_List;                 -- All registered cursors
    Cursor_Count  : C.unsigned;                  -- Total cursor count
    C_Pad1        : Unsigned_32;
  end record
    with Convention => C;
  
  -- Flags --
  DRIVER_WINDOW_BG : constant C.unsigned := 16#02#; -- Managed window background
 
  type Driver_Access is access all Driver with Convention => C;
  subtype Driver_not_null_Access is not null Driver_Access;
  
  ------------------------
  -- Generic Driver Ops --
  ------------------------
  type Open_Func_Access is access function
    (Driver : Driver_not_null_Access;
     Spec   : CS.chars_ptr) return C.int with Convention => C;

  type Close_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Get_Display_Size_Func_Access is access function
    (W,H : access C.unsigned) return C.int with Convention => C;

  --------------------------------
  -- Low-Level Event Processing --
  --------------------------------
  type Begin_Event_Processing_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Pending_Events_Func_Access is access function
    (Driver : Driver_not_null_Access) return C.int with Convention => C;

  type Get_Next_Event_Func_Access is access function
    (Driver : Driver_not_null_Access;
     Event  : Driver_Event_not_null_Access) return C.int with Convention => C;

  type Process_Event_Func_Access is access function
    (Driver : Driver_not_null_Access;
     Event  : Driver_Event_not_null_Access) return C.int with Convention => C;

  type Generic_Event_Loop_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type End_Event_Processing_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Terminate_Func_Access is access procedure with Convention => C;

  -------------------
  -- Rendering Ops --
  -------------------
  type Begin_Rendering_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Render_Window_Func_Access is access procedure
    (Window : Window_not_null_Access) with Convention => C;

  type End_Rendering_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Fill_Rect_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Rect   : SU.Rect_not_null_Access;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Update_Region_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Rect   : SU.Rect_not_null_Access) with Convention => C;

  type Upload_Texture_Func_Access is access procedure
    (Driver   : Driver_not_null_Access;
     Texture  : Texture_Handle_not_null_Access;
     Surface  : SU.Surface_not_null_Access;
     TexCoord : Texture_Coord_Access) with Convention => C;

  type Update_Texture_Func_Access is access function
    (Driver   : Driver_not_null_Access;
     Texture  : C.unsigned;
     Surface  : SU.Surface_not_null_Access;
     TexCoord : Texture_Coord_Access) return C.int with Convention => C;

  type Delete_Texture_Func_Access is access procedure
    (Driver   : Driver_not_null_Access;
     Texture  : C.unsigned) with Convention => C;

  type Set_Refresh_Rate_Func_Access is access function
    (Driver : Driver_not_null_Access;
     FPS    : C.int) return C.int with Convention => C;

  ---------------------------
  -- Clipping and Blending --
  ---------------------------
  type Push_Clip_Rect_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Rect   : SU.Rect_not_null_Access) with Convention => C;

  type Pop_Clip_Rect_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Push_Blending_Mode_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     Source_Fn : SU.Alpha_Func;
     Dest_Fn   : SU.Alpha_Func) with Convention => C;

  type Pop_Blending_Mode_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  ----------------------
  -- Hardware Cursors --
  ----------------------
  type Create_Cursor_Func_Access is access function
    (Driver        : Driver_not_null_Access;
     W,H           : C.unsigned;
     Data, Bitmask : SU.Pixel_Access;
     Hot_X,Hot_Y   : C.int) return Cursor_Access with Convention => C;

  type Free_Cursor_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Cursor : Cursor_not_null_Access) with Convention => C;

  type Set_Cursor_Func_Access is access function
    (Driver : Driver_not_null_Access;
     Cursor : Cursor_not_null_Access) return C.int with Convention => C;

  type Unset_Cursor_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Get_Cursor_Visibility_Func_Access is access function
    (Driver : Driver_not_null_Access) return C.int with Convention => C;

  type Set_Cursor_Visibility_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Enable : C.int) with Convention => C;

  -------------------------
  -- Surfaces / Textures --
  -------------------------
  type Blit_Surface_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Widget  : Widget_not_null_Access;
     Surface : SU.Surface_not_null_Access;
     X,Y     : C.int) with Convention => C;

  type Blit_Surface_From_Func_Access is access procedure
    (Driver   : Driver_not_null_Access;
     Widget   : Widget_not_null_Access;
     Source   : Widget_not_null_Access;
     Surface  : C.int;
     Src_Rect : SU.Rect_Access;
     X,Y      : C.int) with Convention => C;

  type Blit_Surface_GL_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Widget  : Widget_not_null_Access;
     Surface : SU.Surface_not_null_Access;
     W,H     : C.C_float) with Convention => C;

  type Blit_Surface_From_GL_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Widget  : Widget_not_null_Access;
     Surface : C.int;
     W,H     : C.C_float) with Convention => C;

  type Blit_Surface_Flipped_GL_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Widget  : Widget_not_null_Access;
     Surface : C.int;
     W,H     : C.C_float) with Convention => C;

  type Backup_Surfaces_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Widget : Widget_not_null_Access) with Convention => C;

  type Restore_Surfaces_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Widget : Widget_not_null_Access) with Convention => C;

  type Render_to_Surface_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Widget  : Widget_not_null_Access;
     Surface : access SU.Surface_Access) with Convention => C;

  -------------------
  -- Rendering Ops --
  -------------------
  type Put_Pixel_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     X,Y     : C.int;
     Color   : SU.Color_not_null_Access) with Convention => C;

  type Put_Pixel_32_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     X,Y     : C.int;
     Value   : Unsigned_32) with Convention => C;

  type Put_Pixel_RGB8_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     X,Y     : C.int;
     R,G,B   : Unsigned_8) with Convention => C;

#if AG_MODEL = AG_LARGE
  type Put_Pixel_64_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     X,Y     : C.int;
     Value   : Unsigned_64) with Convention => C;

  type Put_Pixel_RGB16_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     X,Y     : C.int;
     R,G,B   : Unsigned_16) with Convention => C;
#end if;

  type Blend_Pixel_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     X,Y       : C.int;
     Color     : SU.Color_not_null_Access;
     Source_Fn : SU.Alpha_Func;
     Dest_Fn   : SU.Alpha_Func) with Convention => C;

  type Draw_Line_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     X1,Y1  : C.int;
     X2,Y2  : C.int;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Horizontal_Line_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     X1,X2  : C.int;
     Y      : C.int;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Vertical_Line_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     X      : C.int;
     Y1,Y2  : C.int;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Blended_Line_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     X1,Y1     : C.int;
     X2,Y2     : C.int;
     Color     : SU.Color_not_null_Access;
     Source_Fn : SU.Alpha_Func;
     Dest_Fn   : SU.Alpha_Func) with Convention => C;
  
  type Draw_Wide_Line_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     X1,Y1     : C.int;
     X2,Y2     : C.int;
     Color     : SU.Color_not_null_Access;
     Width     : C.C_float) with Convention => C;

  type Draw_Wide_Sti16_Line_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     X1,Y1     : C.int;
     X2,Y2     : C.int;
     Color     : SU.Color_not_null_Access;
     Width     : C.C_float;
     Stipple   : Unsigned_16) with Convention => C;

  type Draw_Triangle_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     V1,V2,V3  : SU.AG_Pt_not_null_Access;
     Color     : SU.Color_not_null_Access) with Convention => C;

  type Draw_Polygon_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Points : SU.AG_Pt_not_null_Access;
     Count  : C.unsigned;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Polygon_Sti32_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Points  : SU.AG_Pt_not_null_Access;
     Count   : C.unsigned;
     Color   : SU.Color_not_null_Access;
     Stipple : System.Address) with Convention => C;

  type Draw_Arrow_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Angle   : C.C_float;
     X,Y     : C.int;
     H       : C.int;
     Color_A : SU.Color_not_null_Access;
     Color_B : SU.Color_not_null_Access) with Convention => C;

  type Draw_Box_Rounded_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Rect    : SU.Rect_not_null_Access;
     Z       : C.int;
     Radius  : C.int;
     Color_A : SU.Color_not_null_Access;
     Color_B : SU.Color_not_null_Access) with Convention => C;

  type Draw_Box_Rounded_Top_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Rect    : SU.Rect_not_null_Access;
     Z       : C.int;
     Radius  : C.int;
     Color_A : SU.Color_not_null_Access;
     Color_B : SU.Color_not_null_Access) with Convention => C;

  type Draw_Circle_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     X,Y    : C.int;
     Radius : C.int;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Circle_Filled_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     X,Y    : C.int;
     Radius : C.int;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Rect_Filled_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Rect   : SU.Rect_not_null_Access;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Rect_Blended_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     Rect      : SU.Rect_not_null_Access;
     Color     : SU.Color_not_null_Access;
     Source_Fn : SU.Alpha_Func;
     Dest_Fn   : SU.Alpha_Func) with Convention => C;

  type Draw_Rect_Dithered_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     Rect      : SU.Rect_not_null_Access;
     Color     : SU.Color_not_null_Access) with Convention => C;

  type Update_Glyph_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Glyph  : System.Address) with Convention => C;

  type Draw_Glyph_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Glyph  : System.Address;
     X,Y    : C.int) with Convention => C;

  type Delete_List_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     List   : C.unsigned) with Convention => C;

  --------------------------
  -- Generic Driver Class --
  --------------------------
  type Driver_Class is limited record
    Super       : aliased OBJ.Class;       -- (Object -> Driver)
    Name        : CS.chars_ptr;
    Driver_Type : Driver_Type_t;
    WM_Type     : Driver_WM_Type_t;
#if HAVE_64BIT
    Flags       : Unsigned_64;             -- DRIVER_* flags (below)
#else
    Flags       : C.unsigned;
#end if;
    -- Initialization --
    Open_Func        : Open_Func_Access;
    Close_Func       : Close_Func_Access;
    Get_Display_Size : Get_Display_Size_Func_Access;

    -- Low-level Events --
    Begin_Event_Processing : Begin_Event_Processing_Func_Access;
    Pending_Events         : Pending_Events_Func_Access;
    Get_Next_Event         : Get_Next_Event_Func_Access;
    Process_Event          : Process_Event_Func_Access;
    Generic_Event_Loop     : Generic_Event_Loop_Func_Access;
    End_Event_Processing   : End_Event_Processing_Func_Access;
    Terminate_Func         : Terminate_Func_Access;

    -- Rendering Ops --
    Begin_Rendering  : Begin_Rendering_Func_Access;
    Render_Window    : Render_Window_Func_Access;
    End_Rendering    : End_Rendering_Func_Access;
    Fill_Rect        : Fill_Rect_Func_Access;
    Update_Region    : Update_Region_Func_Access;
    Upload_Texture   : Upload_Texture_Func_Access;
    Update_Texture   : Update_Texture_Func_Access;
    Delete_Texture   : Delete_Texture_Func_Access;
    Set_Refresh_Rate : Set_Refresh_Rate_Func_Access;

    -- Clipping and Blending --
    Push_Clip_Rect     : Push_Clip_Rect_Func_Access;
    Pop_Clip_Rect      : Pop_Clip_Rect_Func_Access;
    Push_Blending_Mode : Push_Blending_Mode_Func_Access;
    Pop_Blending_Mode  : Pop_Blending_Mode_Func_Access;

    -- Hardware Cursors --
    Create_Cursor         : Create_Cursor_Func_Access;
    Free_Cursor           : Free_Cursor_Func_Access;
    Set_Cursor            : Set_Cursor_Func_Access;
    Unset_Cursor          : Unset_Cursor_Func_Access;
    Get_Cursor_Visibility : Get_Cursor_Visibility_Func_Access;
    Set_Cursor_Visibility : Set_Cursor_Visibility_Func_Access;

    -- Surface / Textures --
    Blit_Surface            : Blit_Surface_Func_Access;
    Blit_Surface_From       : Blit_Surface_From_Func_Access;
    Blit_Surface_GL         : Blit_Surface_GL_Func_Access;
    Blit_Surface_From_GL    : Blit_Surface_From_GL_Func_Access;
    Blit_Surface_Flipped_GL : Blit_Surface_Flipped_GL_Func_Access;
    Backup_Surfaces         : Backup_Surfaces_Func_Access;
    Restore_Surfaces        : Restore_Surfaces_Func_Access;
    Render_to_Surface       : Render_to_Surface_Func_Access;

    -- Rendering Ops --
    Put_Pixel            : Put_Pixel_Func_Access;
    Put_Pixel_32         : Put_Pixel_32_Func_Access;
    Put_Pixel_RGB8       : Put_Pixel_RGB8_Func_Access;
#if AG_MODEL = AG_LARGE
    Put_Pixel_64         : Put_Pixel_64_Func_Access;
    Put_Pixel_RGB16      : Put_Pixel_RGB16_Func_Access;
#end if;
    Blend_Pixel          : Blend_Pixel_Func_Access;
    Draw_Line            : Draw_Line_Func_Access;
    Draw_Horizonal_Line  : Draw_Horizontal_Line_Func_Access;
    Draw_Vertical_Line   : Draw_Vertical_Line_Func_Access;
    Draw_Blended_Line    : Draw_Blended_Line_Func_Access;
    Draw_Wide_Line       : Draw_Wide_Line_Func_Access;
    Draw_Wide_Sti16_Line : Draw_Wide_Sti16_Line_Func_Access;
    Draw_Triangle        : Draw_Triangle_Func_Access;
    Draw_Polygon         : Draw_Polygon_Func_Access;
    Draw_Polygon_Sti32   : Draw_Polygon_Sti32_Func_Access;
    Draw_Arrow           : Draw_Arrow_Func_Access;
    Draw_Box_Rounded     : Draw_Box_Rounded_Func_Access;
    Draw_Box_Rounded_Top : Draw_Box_Rounded_Top_Func_Access;
    Draw_Circle          : Draw_Circle_Func_Access;
    Draw_Circle_Filled   : Draw_Circle_Filled_Func_Access;
    Draw_Rect_Filled     : Draw_Rect_Filled_Func_Access;
    Draw_Rect_Blended    : Draw_Rect_Blended_Func_Access;
    Draw_Rect_Dithered   : Draw_Rect_Dithered_Func_Access;
    Update_Glyph         : Update_Glyph_Func_Access;
    Draw_Glyph           : Draw_Glyph_Func_Access;
    Delete_List          : Delete_List_Func_Access;
  end record
    with Convention => C;

  -- Flags --
  DRIVER_OPENGL   : constant C.unsigned := 16#01#;  -- OpenGL supported
  DRIVER_SDL      : constant C.unsigned := 16#02#;  -- SDL 1.x calls supported
  DRIVER_TEXTURES : constant C.unsigned := 16#04#;  -- Texture mgmt supported

  type Driver_Class_Access is access all Driver_Class with Convention => C;
  subtype Driver_Class_not_null_Access is not null Driver_Class_Access;
  
  -----------------------------------------------------------------------------
  --                    D R I V E R -> D R I V E R  M W                      --
  -----------------------------------------------------------------------------

  type Open_Window_Func_Access is access function
    (Window         : Window_not_null_Access;
     Geometry       : SU.Rect_not_null_Access;
     Bits_per_Pixel : C.int;
     Flags          : C.unsigned) return C.int with Convention => C;

  -- Flags --
  DRIVER_MW_ANYPOS       : constant C.unsigned := 16#01#;  -- Autoposition window
  DRIVER_MW_ANYPOS_AVAIL : constant C.unsigned := 16#02#;  -- Autopos supported

  type Close_Window_Func_Access is access procedure
    (Window : Window_not_null_Access) with Convention => C;

  type Map_Window_Func_Access is access function 
    (Window : Window_not_null_Access) return C.int with Convention => C;

  type Unmap_Window_Func_Access is access function 
    (Window : Window_not_null_Access) return C.int with Convention => C;

  type Raise_Window_Func_Access is access function 
    (Window : Window_not_null_Access) return C.int with Convention => C;

  type Lower_Window_Func_Access is access function 
    (Window : Window_not_null_Access) return C.int with Convention => C;

  type Reparent_Window_Func_Access is access function 
    (Window : Window_not_null_Access;
     Parent : Window_not_null_Access;
     X,Y    : C.int) return C.int with Convention => C;

  type Get_Input_Focus_Func_Access is access function 
    (Window : access Window_not_null_Access) return C.int with Convention => C;

  type Set_Input_Focus_Func_Access is access function 
    (Window : Window_not_null_Access) return C.int with Convention => C;

  type Move_Window_Func_Access is access function 
    (Window : Window_not_null_Access;
     X,Y    : C.int) return C.int with Convention => C;

  type Resize_Window_Func_Access is access function 
    (Window : Window_not_null_Access;
     W,H    : C.unsigned) return C.int with Convention => C;

  type Move_Resize_Window_Func_Access is access function 
    (Window   : Window_not_null_Access;
     Geometry : SizeAlloc_Access) return C.int with Convention => C;

  type Pre_Resize_Callback_Func_Access is access procedure
    (Window : Window_not_null_Access) with Convention => C;

  type Post_Resize_Callback_Func_Access is access procedure
    (Window   : Window_not_null_Access;
     Geometry : SizeAlloc_Access) with Convention => C;

  type Capture_Window_Func_Access is access function
    (Window  : Window_not_null_Access;
     Surface : access SU.Surface_Access) return C.int with Convention => C;

  type Set_Border_Width_Func_Access is access function
    (Window : Window_not_null_Access;
     W      : C.unsigned) return C.int with Convention => C;

  type Set_Window_Caption_Func_Access is access function
    (Window  : Window_not_null_Access;
     Caption : CS.chars_ptr) return C.int with Convention => C;

  type Set_Transient_For_Func_Access is access procedure
    (Window, Parent : Window_not_null_Access) with Convention => C;

  type Set_Opacity_Func_Access is access function
    (Window  : Window_not_null_Access;
     Opacity : C.C_float) return C.int with Convention => C;

  type Tweak_Alignment_Func_Access is access procedure
    (Window       : Window_not_null_Access;
     Geometry     : SizeAlloc_Access;
     Max_W, Max_H : C.unsigned) with Convention => C;

  type Driver_MW_Class is limited record
    Super                : aliased Driver_Class;     -- ( Driver -> Driver_MW )
    Open_Window          : Open_Window_Func_Access;
    Close_Window         : Close_Window_Func_Access;
    Map_Window           : Map_Window_Func_Access;
    Unmap_Window         : Unmap_Window_Func_Access;
    Raise_Window         : Raise_Window_Func_Access;
    Lower_Window         : Lower_Window_Func_Access;
    Reparent_Window      : Reparent_Window_Func_Access;
    Get_Input_Focus      : Get_Input_Focus_Func_Access;
    Set_Input_Focus      : Set_Input_Focus_Func_Access;
    Move_Window          : Move_Window_Func_Access;
    Resize_Window        : Resize_Window_Func_Access;
    Move_Resize_Window   : Move_Resize_Window_Func_Access;
    Pre_Resize_Callback  : Pre_Resize_Callback_Func_Access;
    Post_Resize_Callback : Post_Resize_Callback_Func_Access;
    Capture_Window       : Capture_Window_Func_Access;
    Set_Border_Width     : Set_Border_Width_Func_Access;
    Set_Window_Caption   : Set_Window_Caption_Func_Access;
    Set_Transient_For    : Set_Transient_For_Func_Access;
    Set_Opacity          : Set_Opacity_Func_Access;
    Tweak_Alignment      : Tweak_Alignment_Func_Access;
  end record
    with Convention => C;

  type Driver_MW_Class_Access is access all Driver_MW_Class with Convention => C;
  subtype Driver_MW_Class_not_null_Access is not null Driver_MW_Class_Access;
 
  ----------------------------------
  -- Multi-Window Driver Instance --
  ----------------------------------
  type Driver_MW is limited record
    Super  : aliased Driver;          -- ( Driver -> Driver_MW )
    Window : Window_Access;           -- Back reference to window
    Flags  : C.unsigned;              -- DRIVER_MW_* flags (below)
    C_Pad1 : Unsigned_32;
  end record
    with Convention => C;
  
  -- Flags --
  DRIVER_MW_OPEN : constant C.unsigned := 16#01#;  -- Enable rendering

  type Driver_MW_Access is access all Driver_MW with Convention => C;
  subtype Driver_MW_not_null_Access is not null Driver_MW_Access;
  
  -----------------------------------------------------------------------------
  --                    D R I V E R -> D R I V E R  S W                      --
  -----------------------------------------------------------------------------

  type Open_Video_Func_Access is access function
    (Driver         : Driver_not_null_Access;
     W,H            : C.unsigned;
     Bits_per_Pixel : C.int;
     Flags          : C.unsigned) return C.int with Convention => C;

  type Open_Video_Context_Func_Access is access function
    (Driver  : Driver_not_null_Access;
     Context : System.Address;
     Flags   : C.unsigned) return C.int with Convention => C;

  type Set_Video_Context_Func_Access is access function
    (Driver  : Driver_not_null_Access;
     Context : System.Address) return C.int with Convention => C;

  type Close_Video_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;
  
  type Video_Resize_Func_Access is access function
    (Driver : Driver_not_null_Access;
     W,H    : C.unsigned) return C.int with Convention => C;

  type Video_Capture_Func_Access is access function
    (Driver : Driver_not_null_Access) return SU.Surface_Access with Convention => C;
  
  type Video_Clear_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Color  : SU.Color_not_null_Access) with Convention => C;

  --------------------------------
  -- Single-Window Driver Class --
  --------------------------------
  type Driver_SW_Class is limited record
    Super              : aliased Driver_Class;    -- ( Driver -> Driver_SW )
#if HAVE_64BIT
    Flags              : Unsigned_64;             -- DRIVER_SW_* flags (below)
#else
    Flags              : C.unsigned;              -- DRIVER_SW_* flags (below)
#end if;
    Open_Video         : Open_Video_Func_Access;
    Open_Video_Context : Open_Video_Context_Func_Access;
    Set_Video_Context  : Set_Video_Context_Func_Access;
    Close_Video        : Close_Video_Func_Access;
    Video_Resize       : Video_Resize_Func_Access;
    Video_Capture      : Video_Capture_Func_Access;
    Video_Clear        : Video_Clear_Func_Access;
  end record
    with Convention => C;

  type Driver_SW_Class_Access is access all Driver_SW_Class with Convention => C;
  subtype Driver_SW_Class_not_null_Access is not null Driver_SW_Class_Access;
 
  -- Flags --
  DRIVER_SW_OVERLAY    : constant C.unsigned := 16#01#;  -- Overlay mode
  DRIVER_SW_BGPOPUP    : constant C.unsigned := 16#02#;  -- BG popup menu
  DRIVER_SW_FULLSCREEN : constant C.unsigned := 16#04#;  -- Fullscreen mode
  DRIVER_SW_REDRAW     : constant C.unsigned := 16#08#;  -- Redraw queued
  
  type Window_Operation_t is
    (NONE,
     MOVE,
     LEFT_RESIZE,
     RIGHT_RESIZE,
     HORIZ_RESIZE);
  for Window_Operation_t'Size use C.int'Size;

  type Driver_SW is limited record
    Super          : aliased Driver;              -- ( Driver -> Driver_SW )
    W,H            : C.unsigned;                  -- Resolution
    Bits_per_Pixel : C.unsigned;                  -- Depth
    Flags          : C.unsigned;                  -- Flags (none currently)

    Selected_Window      : Window_Access;         -- Window being manipulated
    Last_Key_Down_Window : Window_Access;         -- Window with last kbd event
    Modal_Window_Stack   : System.Address;        -- Modal window list TODO

    Window_Operation     : Window_Operation_t;    -- Window op in progress
    Icon_W, Icon_H       : C.int;                 -- Window icon sizes
    Nominal_FPS          : C.unsigned;            -- Nominal frames/second
    Current_FPS          : C.int;                 -- Last calculated FPS
    BG_Color             : SU.AG_Color;           -- Background color
    Refresh_Last         : C.unsigned;            -- Refresh rate timestamp
#if AG_MODEL = AG_MEDIUM
    C_Pad1               : Unsigned_32;
#end if;
    BG_Popup_Menu        : System.Address;        -- Background popup (TODO)
  end record
    with Convention => C;

  type Driver_SW_Access is access all Driver_SW with Convention => C;
  subtype Driver_SW_not_null_Access is not null Driver_SW_Access;

  ------------------------
  -- Driver Input Event --
  ------------------------
  type Driver_Event_Type is
    (UNKNOWN,
     MOUSE_MOTION,
     MOUSE_BUTTON_DOWN,
     MOUSE_BUTTON_UP,
     MOUSE_ENTER,
     MOUSE_LEAVE,
     FOCUS_IN,
     FOCUS_OUT,
     KEY_DOWN,
     KEY_UP,
     EXPOSE,
     VIDEO_RESIZE,
     CLOSE);
  for Driver_Event_Type'Size use C.int'Size;

  type Driver_Event_Entry is limited record
    Next : Driver_Event_Access;
    Prev : access Driver_Event_Access;
  end record
    with Convention => C;

  type Driver_Event
    (Which : Driver_Event_Type := Driver_Event_Type'First) is
  record
    Event_Type     : Driver_Event_Type;               -- Type of event
    C_Pad1         : Unsigned_32;
    Window         : Window_Access;                   -- For multi-window mode
    Entry_in_Queue : Driver_Event_Entry;              -- Entry in event queue

    case Which is
      when MOUSE_MOTION =>
        X,Y           : C.int;                        -- Mouse coordinates
      when MOUSE_BUTTON_DOWN | MOUSE_BUTTON_UP =>
        Button        : Mouse_Button;                 -- Mouse button index
      when KEY_DOWN | KEY_UP =>
        Keysym        : Key_Sym;                      -- Agar virtual keysym
        Unicode       : Unsigned_32;                  -- Translated Unicode
      when VIDEO_RESIZE =>
        View_X,View_Y : C.int;                        -- New view coordinates
	View_W,View_H : C.int;                        -- New resolution
      when others =>
        null;
    end case;
  end record
    with Convention => C;
  pragma Unchecked_Union (Driver_Event);

  -----------------------------------------------------------------------------
  --               I N P U T  D E V I C E  ->  K E Y B O A R D               --
  -----------------------------------------------------------------------------
  type Keyboard_Keys_Access is access all C.int with Convention => C;
  type Keyboard is limited record
    Super      : aliased INDEV.Input_Device;     -- ( Input_Device -> Keyboard )
    Keys       : Keyboard_Keys_Access;           -- Key state
    Key_Count  : C.unsigned;                     -- Number of keys
    Mod_State  : C.unsigned;                     -- State of modifier keys
  end record
    with Convention => C;

  -----------------------------------------------------------------------------
  --                              W I D G E T                                --
  -----------------------------------------------------------------------------
  type Widget_Private_t is array (1 .. $SIZEOF_AG_WidgetPvt) of
    aliased Unsigned_8 with Convention => C;
  for Widget_Private_t'Size use $SIZEOF_AG_WidgetPvt * System.Storage_Unit;

  type Widget_Surface_Flags_Access is access all Unsigned_8 with Convention => C;
  type Widget is limited record
    Super            : aliased OBJ.Object;  -- ( Object -> Widget )
    Flags            : C.unsigned;          -- WIDGET_* Flags (below)
    X,Y              : C.int;               -- Coordinates in parent widget
    W,H              : C.int;               -- Allocated size in pixels
    Rect             : SU.AG_Rect;          -- Rectangle at 0,0 (cached)
    View_Rect        : SU.AG_Rect2;         -- Display coordinates (cached)
    Sensitivity_Rect : SU.AG_Rect2;         -- Cursor sensitivity rectangle

    Surface_Count    : C.unsigned;                  -- Mapped surface count
    Surfaces         : access SU.Surface_Access;    -- Mapped surfaces
    Surface_Flags    : Widget_Surface_Flags_Access; -- Mapped surface flags
    Textures         : Texture_Handle_Access;       -- Mapped texture handles
    Texcoords        : Texture_Coord_Access;        -- Mapped texture coords

    Forward_Focus_To : Widget_Access;        -- Forward focus to widget
    Parent_Window    : Window_Access;        -- Parent window (if any)
    Driver           : Driver_Access;        -- Parent driver instance
    Driver_Ops       : Driver_Class_Access;  -- Parent driver class
    Stylesheet       : System.Address;       -- TODO Alternate stylesheet
    State            : Widget_State;         -- Style-effecting state
    Margin_Top       : C.int;                -- Margin top (px)
    Margin_Right     : C.int;                -- Margin right (px)
    Margin_Bottom    : C.int;                -- Margin bottom (px)
    Margin_Left      : C.int;                -- Margin left (px)
    Padding_Top      : C.int;                -- Padding top (px)
    Padding_Right    : C.int;                -- Padding right (px)
    Padding_Bottom   : C.int;                -- Padding bottom (px)
    Padding_Left     : C.int;                -- Padding left (px)
    Spacing_Horiz    : C.int;                -- Spacing top (px)
    Spacing_Vert     : C.int;                -- Spacing right (px)
    Font             : System.Address;       -- Active font (TODO)
    Palette          : Widget_Palette;       -- Color palette
#if HAVE_OPENGL
    GL_Context       : Widget_GL_Context_Access; -- Context for USE_OPENGL
#end if;
    Actions_Data     : System.Address;       -- TODO
    Actions_Length   : C.int;                -- TODO
    Actions_Capacity : C.int;                -- TODO
    Private_Data     : Widget_Private_t;
  end record
    with Convention => C;
  
  -- Flags --
  WIDGET_FOCUSABLE            : constant C.unsigned := 16#00_0001#; -- Can grab focus
  WIDGET_FOCUSED              : constant C.unsigned := 16#00_0002#; -- Holds focus (read-only)
  WIDGET_UNFOCUSED_MOTION     : constant C.unsigned := 16#00_0004#; -- Receive mousemotion regardless of focus
  WIDGET_UNFOCUSED_BUTTONUP   : constant C.unsigned := 16#00_0008#; -- Receive buttonup regardless of focus
  WIDGET_UNFOCUSED_BUTTONDOWN : constant C.unsigned := 16#00_0010#; -- Receive buttondown regardless of focus
  WIDGET_VISIBLE              : constant C.unsigned := 16#00_0020#; -- Is visible (read-only)
  WIDGET_H_FILL               : constant C.unsigned := 16#00_0040#; -- Expand to fill width
  WIDGET_V_FILL               : constant C.unsigned := 16#00_0080#; -- Expand to fill height
  WIDGET_USE_OPENGL           : constant C.unsigned := 16#00_0100#; -- Set up separate GL context
  WIDGET_HIDE                 : constant C.unsigned := 16#00_0200#; -- Don't draw
  WIDGET_DISABLED             : constant C.unsigned := 16#00_0400#; -- Disabled state, ignore input
  WIDGET_MOUSEOVER            : constant C.unsigned := 16#00_0800#; -- Mouseover state (read-only)
  WIDGET_CATCH_TAB            : constant C.unsigned := 16#00_1000#; -- Receive events for focus-cycling key
  WIDGET_GL_RESHAPE           : constant C.unsigned := 16#00_2000#; -- Pending reshape
  WIDGET_UNDERSIZE            : constant C.unsigned := 16#00_4000#; -- Too small to draw
  WIDGET_NO_SPACING           : constant C.unsigned := 16#00_8000#; -- Ignore box model
  WIDGET_UNFOCUSED_KEYDOWN    : constant C.unsigned := 16#01_0000#; -- Receive keydowns regardless of focus
  WIDGET_UNFOCUSED_KEYUP      : constant C.unsigned := 16#02_0000#; -- Receive keyups regardless of focus
  WIDGET_UPDATE_WINDOW        : constant C.unsigned := 16#10_0000#; -- Request Window_Update ASAP
  WIDGET_QUEUE_SURFACE_BACKUP : constant C.unsigned := 16#20_0000#; -- Backup surfaces ASAP
  WIDGET_USE_TEXT             : constant C.unsigned := 16#40_0000#; -- Use font engine
  WIDGET_USE_MOUSEOVER        : constant C.unsigned := 16#80_0000#; -- Generate mouseover events
  WIDGET_EXPAND               : constant C.unsigned := WIDGET_H_FILL or
                                                       WIDGET_V_FILL;
  -- Surface flags --
  WIDGET_SURFACE_NODUP : constant C.unsigned := 16#01#; -- Don't free on cleanup
  WIDGET_SURFACE_REGEN : constant C.unsigned := 16#02#; -- Regen texture ASAP
 
  ------------------------------
  -- Boolean Flag Description --
  ------------------------------
  type Flag_Descr is limited record
#if HAVE_64BIT
    Bitmask   : Unsigned_64;            -- Bitmask
#else
    Bitmask   : C.unsigned;             -- Bitmask
#end if;
    Text      : CS.chars_ptr;           -- Description (UTF-8)
    Writeable : C.int;                  -- User-editable
    C_Pad1    : Unsigned_32;
  end record
    with Convention => C;
  type Flag_Descr_Access is access all Flag_Descr with Convention => C;
  subtype Flag_Descr_not_null_Access is not null Flag_Descr_Access;
  
  --------------------
  -- Widget Actions --
  --------------------
  type Action_Type is
    (ACTION_FN,                 -- Call subroutine
     ACTION_SET_INT,            -- Set an integer 0 or 1
     ACTION_TOGGLE_INT,         -- Toggle an integer
     ACTION_SET_FLAG,           -- Set bit(s) 0 or 1
     ACTION_TOGGLE_FLAG);       -- Toggle bit(s)
  for Action_Type'Size use C.int'Size;

  type Action_Event_Type is
    (ACTION_BUTTON_DOWN,        -- Button pressed
     ACTION_BUTTON_UP,          -- Button released
     ACTION_KEY_DOWN,           -- Key pressed (once)
     ACTION_KEY_UP,             -- Key released
     ACTION_KEY_REPEAT);        -- Key pressed (with key repeat)
  for Action_Event_Type'Size use C.int'Size;

  type Action is limited record
    Act_Type     : Action_Type;             -- Type of action
    C_Pad1       : Unsigned_32;
    Widget       : Widget_Access;           -- Back pointer to widget
    Func         : EV.Event_Access;         -- Callback routine
    Set_Target   : System.Address;          -- Target for SET_{INT,FLAG}
    Set_Value    : C.int;                   -- Value for SET_INT
    Flag_Bitmask : C.unsigned;              -- Bitmask for {SET,TOGGLE}_FLAG
  end record
    with Convention => C;
  type Action_Access is access all Action with Convention => C;
  subtype Action_not_null_Access is not null Action_Access;

  type Action_Tie is array (1 .. $SIZEOF_AG_ActionTie)
    of aliased Unsigned_8 with Convention => C;
  for Action_Tie'Size use $SIZEOF_AG_ActionTie * System.Storage_Unit;
  type Action_Tie_Access is access all Action_Tie with Convention => C;
  subtype Action_Tie_not_null_Access is not null Action_Tie_Access;
  
  type Redraw_Tie is array (1 .. $SIZEOF_AG_RedrawTie)
    of aliased Unsigned_8 with Convention => C;
  for Redraw_Tie'Size use $SIZEOF_AG_RedrawTie * System.Storage_Unit;
  type Redraw_Tie_Access is access all Redraw_Tie with Convention => C;
  subtype Redraw_Tie_not_null_Access is not null Redraw_Tie_Access;
 
  ------------------------
  -- Cursor-Change Area --
  ------------------------
  type Cursor_Area is array (1 .. $SIZEOF_AG_CursorArea)
    of aliased Unsigned_8 with Convention => C;
  for Cursor_Area'Size use $SIZEOF_AG_CursorArea * System.Storage_Unit;
  type Cursor_Area_Access is access all Cursor_Area with Convention => C;
  subtype Cursor_Area_not_null_Access is not null Cursor_Area_Access;
  
  -----------------------------------------------------------------------------
  --                              W I N D O W                                --
  -----------------------------------------------------------------------------

  CAPTION_MAX : constant Natural := $AG_WINDOW_CAPTION_MAX;

  type WM_Function is
    (WM_NORMAL,            -- Normal top-level window
     WM_DESKTOP,           -- Desktop feature (e.g., full-screen)
     WM_DOCK,              -- Dock or panel feature
     WM_TOOLBAR,           -- Toolbar torn off from main window
     WM_MENU,              -- Pinnable menu window
     WM_UTILITY,           -- Persistent utility window (palette, toolbox)
     WM_SPLASH,            -- Introductory screen
     WM_DIALOG,            -- Dialog window
     WM_DROPDOWN_MENU,     -- Menubar-triggered drop-down menu
     WM_POPUP_MENU,        -- Contextual popup menu
     WM_TOOLTIP,           -- Mouse hover triggered tooltip
     WM_NOTIFICATION,      -- Notification bubble
     WM_COMBO,             -- Combo-box triggered window
     WM_DND);              -- Draggable object

  for WM_Function'Size use C.int'Size;
 
  type Window_Alignment is
    (NO_ALIGNMENT,
     TOP_LEFT,      TOP_CENTER,      TOP_RIGHT,
     MIDDLE_LEFT,   MIDDLE_CENTER,   MIDDLE_RIGHT,
     BOTTOM_LEFT,   BOTTOM_CENTER,   BOTTOM_RIGHT,
     LAST_ALIGNMENT);

  for Window_Alignment'Size use C.int'Size;

  type Window_Caption is array (1 .. CAPTION_MAX) of
    aliased C.char with Convention => C;
  type Window_Private_t is array (1 .. $SIZEOF_AG_WindowPvt) of
    aliased Unsigned_8 with Convention => C;
  for Window_Private_t'Size use $SIZEOF_AG_WindowPvt * System.Storage_Unit;

  type Entry_in_User_t is limited record
    Next : Window_Access;
    Prev : access Window_Access;
  end record
    with Convention => C;

  type Window is limited record
    Super                : aliased Widget;    -- ( Widget -> Window )
    Flags                : C.unsigned;        -- WINDOW_* flags (below)
    Caption              : Window_Caption;    -- Window title
    Visible              : C.int;             -- Visibility flag
    Dirty                : C.int;             -- Redraw flag
    Alignment            : Window_Alignment;  -- Initial position
    Title_Bar            : System.Address;    -- TODO AG_Titlebar
    Icon                 : System.Address;    -- TODO AG_Icon
    C_Pad1               : Unsigned_32;
    Min_W, Min_H         : C.int;             -- Minimum window size
    Bottom_Border_W      : C.int;             -- Bottom border width (px)
    Side_Borders_W       : C.int;             -- Side borders width (px)
    Resize_Control_W     : C.int;             -- Resize control width (px)
    Rect                 : SU.AG_Rect;        -- Effective view rectangle
    Rect_Saved           : SU.AG_Rect;        -- For Window Restore operation
    Min_Size_Pct         : C.int;             -- Size in % for MINSIZE_IS_PCT
    Focused_Widget_Count : C.int;             -- Number of focused widgets
    Excl_Motion_Widget   : Widget_Access;     -- Hog all mousemotion events
    Window_Function      : WM_Function;       -- High-level WM function
    Zoom_Pct             : C.int;             -- Effective zoom level in %
    Parent_Window        : Window_Access;     -- Parent window
    Transient_For_Window : Window_Access;     -- Is transient for that window
    Pinned_To_Window     : Window_Access;     -- Is pinned to that window
    Entry_in_User        : Entry_in_User_t;   -- In optional user linked list
    Private_Data         : Window_Private_t;
  end record
    with Convention => C;
  
  -- Flags --
  WINDOW_MODAL            : constant C.unsigned := 16#0000_0001#; -- App modal
  WINDOW_MAXIMIZED        : constant C.unsigned := 16#0000_0002#; -- Maximized
  WINDOW_MINIMIZED        : constant C.unsigned := 16#0000_0004#; -- Minimized
  WINDOW_KEEP_ABOVE       : constant C.unsigned := 16#0000_0008#; -- Keep above others
  WINDOW_KEEP_BELOW       : constant C.unsigned := 16#0000_0010#; -- Keep below others
  WINDOW_DENY_FOCUS       : constant C.unsigned := 16#0000_0020#; -- Reject focus
  WINDOW_NO_TITLE         : constant C.unsigned := 16#0000_0040#; -- No titlebar
  WINDOW_NO_BORDERS       : constant C.unsigned := 16#0000_0080#; -- No borders
  WINDOW_NO_H_RESIZE      : constant C.unsigned := 16#0000_0100#; -- No horiz resize
  WINDOW_NO_V_RESIZE      : constant C.unsigned := 16#0000_0200#; -- No vert resize
  WINDOW_NO_CLOSE         : constant C.unsigned := 16#0000_0400#; -- No close button
  WINDOW_NO_MINIMIZE      : constant C.unsigned := 16#0000_0800#; -- No minimize button
  WINDOW_NO_MAXIMIZE      : constant C.unsigned := 16#0000_1000#; -- No maximize button
  WINDOW_TILEABLE         : constant C.unsigned := 16#0000_2000#; -- WM can tile
  WINDOW_MINSIZE_IS_PCT   : constant C.unsigned := 16#0000_4000#; -- Min size is in %
  WINDOW_NO_BACKGROUND    : constant C.unsigned := 16#0000_8000#; -- No bg fill
  WINDOW_MAIN             : constant C.unsigned := 16#0001_0000#; -- Exit when closed
  WINDOW_FOCUS_ON_ATTACH  : constant C.unsigned := 16#0002_0000#; -- Focus after attach
  WINDOW_H_MAXIMIZE       : constant C.unsigned := 16#0004_0000#; -- Keep maximized horizontally
  WINDOW_V_MAXIMIZE       : constant C.unsigned := 16#0008_0000#; -- Keep maximized vertically
  WINDOW_NO_MOVE          : constant C.unsigned := 16#0010_0000#; -- Disable movement
  WINDOW_NO_CLIPPING      : constant C.unsigned := 16#0020_0000#; -- No clipping rectangle over window
  WINDOW_MODKEY_EVENTS    : constant C.unsigned := 16#0040_0000#; -- Modifier keys generate keyup/keydown
  WINDOW_DETACHING        : constant C.unsigned := 16#0080_0000#; -- Detach in progress (read-only)
  WINDOW_NO_CURSOR_CHANGE : constant C.unsigned := 16#0400_0000#; -- Disable cursor updates
  WINDOW_FADE_IN          : constant C.unsigned := 16#0800_0000#; -- Fade in (if supported)
  WINDOW_FADE_OUT         : constant C.unsigned := 16#1000_0000#; -- Fade out (if supported)
  WINDOW_NO_RESIZE        : constant C.unsigned := WINDOW_NO_H_RESIZE or
                                                   WINDOW_NO_V_RESIZE;
  WINDOW_NO_BUTTONS       : constant C.unsigned := WINDOW_NO_CLOSE or
                                                   WINDOW_NO_MINIMIZE or
						   WINDOW_NO_MAXIMIZE;
  WINDOW_PLAIN            : constant C.unsigned := WINDOW_NO_TITLE or
                                                   WINDOW_NO_BORDERS;

  ----------------
  -- Widget API --
  ----------------

  --
  -- Return the first visible widget intersecting a point or enclosing a
  -- rectangle (in view coordinates). Scan all drivers and return first match.
  --
  --function Find_At_Point
  --  (Class : in String;
  --   X,Y   : in Natural) return Widget_Access;
  --function Find_Enclosing_Rect
  --  (Class : in String;
  --   X,Y   : in Natural;
  --   W,H   : in Positive) return Widget_Access;
 
  --
  -- Create / destroy a low-level driver instance.
  -- 
  function Open_Driver (Class : Driver_Class_not_null_Access) return Driver_Access
    with Import, Convention => C, Link_Name => "AG_DriverOpen";
  procedure Close_Driver (Driver : Driver_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_DriverClose";

  --
  -- Return the driver instance for the given numerical ID.
  --
  function Get_Driver (Driver_ID : C.unsigned) return Driver_Access
    with Import, Convention => C, Link_Name => "ag_get_driver_by_id";

  --
  -- Dump video memory to a jpeg file in ~/.<progname>/screenshot/.
  --
  procedure Capture_Screenshot
    with Import, Convention => C, Link_Name => "AG_ViewCapture";

  --
  -- Render a widget to the display.
  -- Context: Low-level rendering (i.e., AG_Driver(3) code)
  --
  procedure Draw (Widget : in Widget_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetDraw";
 
  --
  -- Request a widget's preferred initial size.
  -- Context: Container widget's Size_Request or Size_Allocate operation.
  --
  procedure Size_Req
    (Widget : in Widget_not_null_Access;
     Size   : in SizeReq_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetSizeReq";
  
  --
  -- Allocate an effective widget size and position.
  -- Context: Container widget's Size_Request or Size_Allocate operation.
  --
  procedure Size_Alloc
    (Widget : in Widget_not_null_Access;
     Size   : in SizeAlloc_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetSizeAlloc";

  --
  -- Set whether to accept (or deny) focused state.
  --
  procedure Set_Focusable
    (Widget : in Widget_not_null_Access;
     Enable : in Boolean);
  function Set_Focusable
    (Widget : in Widget_not_null_Access;
     Enable : in Boolean) return Boolean;
 
  --
  -- Arrange for focus state to be forwarded automatically to the given
  -- Target widget (or null = disable focus forwarding).
  --
  procedure Forward_Focus
    (Widget : in Widget_not_null_Access;
     Target : in Widget_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetForwardFocus";

  --
  -- Focus on the widget (and implicitely its parents up to and including
  -- the parent window).
  --
  procedure Focus
    (Widget : in Widget_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetFocus";
  function Focus
    (Widget : in Widget_not_null_Access) return Boolean;
 
  --
  -- Remove focus state from the widget (and its children implicitely).
  --
  procedure Unfocus
    (Widget : in Widget_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetUnfocus";
 
  --
  -- Search for a focused widget under Root (which can also be a Window).
  --
  function Find_Focused_Widget
    (Root : in Widget_not_null_Access) return Widget_Access
    with Import, Convention => C, Link_Name => "AG_WidgetFindFocused";
 
  --
  -- Return the topmost visible widget intersecting a display-coordinate point.
  --
  function Find_Widget_At_Point
    (Class : String;
     X,Y   : Natural) return Widget_Access;

  --
  -- Return topmost visible widget enclosing a display-coordinate rectangle.
  --
  function Find_Widget_Enclosing_Rect
    (Class         : String;
     X,Y           : Natural;
     Width, Height : Positive) return Widget_Access;

  --
  -- Update the effective view coordinates of a widget and its descendants.
  --
  procedure Update_Coordinates
    (Widget : in Widget_not_null_Access;
     X      : in Natural;
     Y      : in Natural);

  ------------------------
  -- Widget Surface API --
  ------------------------

  --
  -- Attach a Surface to a Widget such that:
  --
  --   1) It is freed automatically when the widget is destroyed.
  --   2) A hardware texture is generated automatically for it
  --      (where supported by the graphics backend).
  --
  -- Returned handle is unique to the Widget (and is index into its internal
  -- Surfaces, Surface_Flags, Textures and Texcoords arrays).
  -- 
  function Map_Surface
    (Widget  : in Widget_not_null_Access;
     Surface : in SU.Surface_not_null_Access) return Surface_Handle
    with Import, Convention => C, Link_Name => "AG_WidgetMapSurface";

  --
  -- Delete and replace any Surface corresponding to the given handle (as
  -- returned by Map_Surface). The previous surface (if any) is freed, and
  -- any associated hardware texture is regenerated.
  --
  -- Passing Surface => null is equivalent to calling Unmap_Surface.
  --
  procedure Replace_Surface
    (Widget  : in Widget_not_null_Access;
     Handle  : in Surface_Handle;
     Surface : in SU.Surface_Access := null)
    with Import, Convention => C, Link_Name => "AG_WidgetReplaceSurface";
 
  --
  -- Free any Surface mapped to the given handle (as returned by Map_Surface).
  -- Delete any hardware texture associated with the surface.
  --
  procedure Unmap_Surface
    (Widget : in Widget_not_null_Access;
     Handle : in Surface_Handle);

  --
  -- Blit the surface (or render the hardware texture) at Source:[Handle],
  -- at target coordinates X,Y relative to Widget.
  --
  -- Source may be different from Widget (i.e., Widgets may render other
  -- widgets' surfaces) as long as both widgets are in the same Window.
  --
  procedure Blit_Surface
    (Widget   : in Widget_not_null_Access;
     Source   : in Widget_not_null_Access;
     Handle   : in Surface_Handle;
     Src_Rect : in SU.Rect_Access := null;
     X,Y      : in Natural := 0);

  --
  -- Blit the surface (or render the hardware texture) at Widget:[Handle],
  -- at target coordinates X,Y relative to Widget.
  --
  procedure Blit_Surface
    (Widget   : in Widget_not_null_Access;
     Handle   : in Surface_Handle;
     Src_Rect : in SU.Rect_Access := null;
     X,Y      : in Natural := 0);

  --
  -- Blit a Surface not managed by the Widget. This method is inefficient
  -- (no hardware acceleration) and should be avoided.
  --
  procedure Blit_Surface
    (Widget   : in Widget_not_null_Access;
     Surface  : in SU.Surface_not_null_Access;
     X,Y      : in Natural := 0);

#if HAVE_OPENGL
  --
  -- Coordinate-free variants of Blit_Surface for OpenGL-only widgets.
  -- Rely on GL transformations instead of coordinates.
  --
  procedure Blit_Surface_GL
    (Widget        : in Widget_not_null_Access;
     Handle        : in Surface_Handle;
     Width, Height : in C.C_float := 1.0)
    with Import, Convention => C, Link_Name => "AG_WidgetBlitSurfaceGL";

  procedure Blit_Surface_GL
    (Widget        : in Widget_not_null_Access;
     Surface       : in SU.Surface_not_null_Access;
     Width, Height : in C.C_float := 1.0)
    with Import, Convention => C, Link_Name => "AG_WidgetBlitGL";

  procedure Blit_Surface_GL_Flipped
    (Widget        : in Widget_not_null_Access;
     Surface       : in SU.Surface_not_null_Access;
     Width, Height : in C.C_float := 1.0)
    with Import, Convention => C, Link_Name => "AG_WidgetBlitSurfaceFlippedGL";

  --
  -- Destroy all GL resources associated with a widget and its children
  -- (but in a way that allows us to regenerate the GL context later).
  --
  procedure Free_GL_Resources (Widget : in Widget_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetFreeResourcesGL";
  
  --
  -- Regenerate GL resources associated with a widget after loss of GL context.
  --
  procedure Regen_GL_Resources (Widget : in Widget_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetRegenResourcesGL";
#end if;

  --
  -- Test whether widget is sensitive to view coordinates X,Y.
  --
  function Is_Sensitive
    (Widget : in Widget_not_null_Access;
     X,Y    : in Natural) return Boolean;

  ---------------
  -- Mouse API --
  ---------------

  --
  -- Create a new mouse device instance.
  --
  function New_Mouse
    (Driver : in Driver_not_null_Access;
     Descr  : in String) return Mouse_not_null_Access;

  --
  -- Change the cursor if its coordinates overlap a registered cursor area.
  -- Generally called from window/driver code following a mouse motion event.
  --
  procedure Mouse_Cursor_Update
    (Window : in Window_not_null_Access;
     X,Y    : in Natural);

  --
  -- Handle a mouse motion.
  -- Called from Driver code (agDrivers must be locked).
  --
  procedure Process_Mouse_Motion
    (Window    : in Window_not_null_Access;
     X,Y       : in Natural;
     Xrel,Yrel : in Integer;
     Buttons   : in Mouse_Button);

  --
  -- Handle a mouse button press / release.
  -- Called from Driver code (agDrivers must be locked).
  --
  procedure Process_Mouse_Button_Up
    (Window : in Window_not_null_Access;
     X,Y    : in Natural;
     Button : in Mouse_Button);
  procedure Process_Mouse_Button_Down
    (Window : in Window_not_null_Access;
     X,Y    : in Natural;
     Button : in Mouse_Button);

  --
  -- Clear the internal cache of rendered glyphs.
  --
  procedure Clear_Glyph_Cache
    (Driver : in Driver_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TextClearGlyphCache";

  ------------------
  -- Keyboard API --
  ------------------

  --
  -- Create a new keyboard device instance.
  --
  function New_Keyboard
    (Driver : in Driver_not_null_Access;
     Descr  : in String) return Keyboard_not_null_Access;

  private
 
  -- gui/widget.c

  function AG_WidgetSetFocusable
    (Widget : in Widget_not_null_Access;
     Enable : in C.int) return C.int
    with Import, Convention => C, Link_Name => "AG_WidgetSetFocusable";

  function AG_WidgetFocus
    (Widget : in Widget_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_WidgetFocus";

  function AG_WidgetFindPoint
    (Class : in CS.chars_ptr;
     X,Y   : in C.int) return Widget_Access
    with Import, Convention => C, Link_Name => "AG_WidgetFindPoint";
  
  function AG_WidgetFindRect
    (Class  : in CS.chars_ptr;
     X,Y    : in C.int;
     Width  : in C.int;
     Height : in C.int) return Widget_Access
    with Import, Convention => C, Link_Name => "AG_WidgetFindRect";
  
  procedure AG_WidgetUpdateCoords
    (Widget : in Widget_not_null_Access;
     X,Y    : in C.int)
    with Import, Convention => C, Link_Name => "AG_WidgetUpdateCoords";
 
  procedure AG_WidgetBlitFrom
    (Widget   : in Widget_not_null_Access;
     Source   : in Widget_not_null_Access;
     Handle   : in Surface_Handle;
     Src_Rect : in SU.Rect_Access;
     X,Y      : in C.int)
    with Import, Convention => C, Link_Name => "ag_widget_blit_from";
  
  procedure AG_WidgetBlit
    (Widget   : in Widget_not_null_Access;
     Surface  : in SU.Surface_not_null_Access;
     X,Y      : in C.int)
    with Import, Convention => C, Link_Name => "ag_widget_blit";

  function AG_WidgetSensitive
    (Widget : in Widget_not_null_Access;
     X,Y    : in C.int) return C.int
    with Import, Convention => C, Link_Name => "AG_WidgetSensitive";

  procedure AG_MouseCursorUpdate
    (Window : Window_not_null_Access;
     X,Y    : C.int)
    with Import, Convention => C, Link_Name => "AG_MouseCursorUpdate";
  
  procedure AG_ProcessMouseMotion
    (Window    : Window_not_null_Access;
     X,Y       : C.int;
     Xrel,Yrel : C.int;
     Buttons   : Mouse_Button)
    with Import, Convention => C, Link_Name => "AG_ProcessMouseMotion";

  procedure AG_ProcessMouseButtonUp
    (Window    : Window_not_null_Access;
     X,Y       : C.int;
     Button    : Mouse_Button)
    with Import, Convention => C, Link_Name => "AG_ProcessMouseButtonUp";

  procedure AG_ProcessMouseButtonDown
    (Window    : Window_not_null_Access;
     X,Y       : C.int;
     Button    : Mouse_Button)
    with Import, Convention => C, Link_Name => "AG_ProcessMouseButtonDown";

  -- gui/keyboard.c

  function AG_KeyboardNew
    (Driver : in Driver_not_null_Access;
     Descr  : in CS.chars_ptr) return Keyboard_not_null_Access
    with Import, Convention => C, Link_Name => "AG_KeyboardNew";

  -- gui/mouse.c
    --
  function AG_MouseNew
    (Driver : in Driver_not_null_Access;
     Descr  : in CS.chars_ptr) return Mouse_not_null_Access
    with Import, Convention => C, Link_Name => "AG_MouseNew";

end Agar.Widget;
