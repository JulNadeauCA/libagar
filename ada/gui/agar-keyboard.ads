------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                        A G A R  . K E Y B O A R D                        --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces.C;
with Interfaces.C.Strings;
with Agar.Input_Device;
with Agar.Types; use Agar.Types;

package Agar.Keyboard is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;
  package INDEV renames Agar.Input_Device;

  use type C.int;
  use type C.unsigned;

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

  ---------------------
  -- Keyboard Device --
  ---------------------
  type Keyboard_Keys_Access is access all C.int with Convention => C;
  type Keyboard_Device is limited record
    Super      : aliased INDEV.Input_Device;     -- [Input_Device -> Keyboard]
    Keys       : Keyboard_Keys_Access;           -- Key state
    Key_Count  : C.unsigned;                     -- Number of keys
    Mod_State  : C.unsigned;                     -- State of modifier keys
  end record
    with Convention => C;

  type Keyboard_Device_Access is access all Keyboard_Device with Convention => C;
  subtype Keyboard_Device_not_null_Access is not null Keyboard_Device_Access;

end Agar.Keyboard;
