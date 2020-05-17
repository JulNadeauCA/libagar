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
     KEY_LAST,
     KEY_ANY);
  for Key_Sym use
    (KEY_NONE          => 16#00_00#,
     KEY_BACKSPACE     => 16#00_08#,
     KEY_TAB           => 16#00_09#,
     KEY_CLEAR         => 16#00_0c#,
     KEY_RETURN        => 16#00_0d#,
     KEY_PAUSE         => 16#00_13#,
     KEY_ESCAPE        => 16#00_1b#,
     KEY_SPACE         => 16#00_20#,    --   --
     KEY_EXCLAIM       => 16#00_21#,	-- ! --
     KEY_QUOTEDBL      => 16#00_22#,	-- " --
     KEY_HASH          => 16#00_23#,	-- # --
     KEY_DOLLAR        => 16#00_24#,	-- $ --
     KEY_PERCENT       => 16#00_25#,	-- % --
     KEY_AMPERSAND     => 16#00_26#,	-- & --
     KEY_QUOTE         => 16#00_27#,	-- ' --
     KEY_LEFT_PAREN    => 16#00_28#,	-- ( --
     KEY_RIGHT_PAREN   => 16#00_29#,	-- ) --
     KEY_ASTERISK      => 16#00_2a#,	-- * --
     KEY_PLUS          => 16#00_2b#,	-- + --
     KEY_COMMA         => 16#00_2c#,	-- , --
     KEY_MINUS         => 16#00_2d#,	-- - --
     KEY_PERIOD        => 16#00_2e#,	-- . --
     KEY_SLASH         => 16#00_2f#,	-- / --
     KEY_0             => 16#00_30#,	-- 0 --
     KEY_1             => 16#00_31#,	-- 1 --
     KEY_2             => 16#00_32#,	-- 2 --
     KEY_3             => 16#00_33#,	-- 3 --
     KEY_4             => 16#00_34#,	-- 4 --
     KEY_5             => 16#00_35#,	-- 5 --
     KEY_6             => 16#00_36#,	-- 6 --
     KEY_7             => 16#00_37#,	-- 7 --
     KEY_8             => 16#00_38#,	-- 8 --
     KEY_9             => 16#00_39#,	-- 9 --
     KEY_COLON         => 16#00_3a#,	-- : --
     KEY_SEMICOLON     => 16#00_3b#,	-- ; --
     KEY_LESS          => 16#00_3c#,	-- < --
     KEY_EQUALS        => 16#00_3d#,	-- = --
     KEY_GREATER       => 16#00_3e#,	-- > --
     KEY_QUESTION      => 16#00_3f#,	-- ? --
     KEY_AT            => 16#00_40#,	-- @ --
     KEY_LEFT_BRACKET  => 16#00_5b#,	-- [ --
     KEY_BACKSLASH     => 16#00_5c#,	-- \ --
     KEY_RIGHT_BRACKET => 16#00_5d#,	-- ] --
     KEY_CARET         => 16#00_5e#,	-- ^ --
     KEY_UNDERSCORE    => 16#00_5f#,	-- _ --
     KEY_BACKQUOTE     => 16#00_60#,	-- ` --
     KEY_A             => 16#00_61#,	-- a --
     KEY_B             => 16#00_62#,	-- b --
     KEY_C             => 16#00_63#,	-- c --
     KEY_D             => 16#00_64#,	-- d --
     KEY_E             => 16#00_65#,	-- e --
     KEY_F             => 16#00_66#,	-- f --
     KEY_G             => 16#00_67#,	-- g --
     KEY_H             => 16#00_68#,	-- h --
     KEY_I             => 16#00_69#,	-- i --
     KEY_J             => 16#00_6a#,	-- j --
     KEY_K             => 16#00_6b#,	-- k --
     KEY_L             => 16#00_6c#,	-- l --
     KEY_M             => 16#00_6d#,	-- m --
     KEY_N             => 16#00_6e#,	-- n --
     KEY_O             => 16#00_6f#,	-- o --
     KEY_P             => 16#00_70#,	-- p --
     KEY_Q             => 16#00_71#,	-- q --
     KEY_R             => 16#00_72#,	-- r --
     KEY_S             => 16#00_73#,	-- s --
     KEY_T             => 16#00_74#,	-- t --
     KEY_U             => 16#00_75#,	-- u --
     KEY_V             => 16#00_76#,	-- v --
     KEY_W             => 16#00_77#,	-- w --
     KEY_X             => 16#00_78#,	-- x --
     KEY_Y             => 16#00_79#,	-- y --
     KEY_Z             => 16#00_7a#,	-- z --
     KEY_DELETE        => 16#00_7f#,
     KEY_KP0           => 16#01_00#,
     KEY_KP1           => 16#01_01#,
     KEY_KP2           => 16#01_02#,
     KEY_KP3           => 16#01_03#,
     KEY_KP4           => 16#01_04#,
     KEY_KP5           => 16#01_05#,
     KEY_KP6           => 16#01_06#,
     KEY_KP7           => 16#01_07#,
     KEY_KP8           => 16#01_08#,
     KEY_KP9           => 16#01_09#,
     KEY_KP_PERIOD     => 16#01_0a#,
     KEY_KP_DIVIDE     => 16#01_0b#,
     KEY_KP_MULTIPLY   => 16#01_0c#,
     KEY_KP_MINUS      => 16#01_0d#,
     KEY_KP_PLUS       => 16#01_0e#,
     KEY_KP_ENTER      => 16#01_0f#,
     KEY_KP_EQUALS     => 16#01_10#,
     KEY_UP            => 16#01_11#,
     KEY_DOWN          => 16#01_12#,
     KEY_RIGHT         => 16#01_13#,
     KEY_LEFT          => 16#01_14#,
     KEY_INSERT        => 16#01_15#,
     KEY_HOME          => 16#01_16#,
     KEY_END           => 16#01_17#,
     KEY_PAGE_UP       => 16#01_18#,
     KEY_PAGE_DOWN     => 16#01_19#,
     KEY_F1            => 16#01_1a#,
     KEY_F2            => 16#01_1b#,
     KEY_F3            => 16#01_1c#,
     KEY_F4            => 16#01_1d#,
     KEY_F5            => 16#01_1e#,
     KEY_F6            => 16#01_1f#,
     KEY_F7            => 16#01_20#,
     KEY_F8            => 16#01_21#,
     KEY_F9            => 16#01_22#,
     KEY_F10           => 16#01_23#,
     KEY_F11           => 16#01_24#,
     KEY_F12           => 16#01_25#,
     KEY_F13           => 16#01_26#,
     KEY_F14           => 16#01_27#,
     KEY_F15           => 16#01_28#,
     KEY_NUM_LOCK      => 16#01_2c#,
     KEY_CAPS_LOCK     => 16#01_2d#,
     KEY_SCROLL_LOCK   => 16#01_2e#,
     KEY_RIGHT_SHIFT   => 16#01_2f#,
     KEY_LEFT_SHIFT    => 16#01_30#,
     KEY_RIGHT_CTRL    => 16#01_31#,
     KEY_LEFT_CTRL     => 16#01_32#,
     KEY_RIGHT_ALT     => 16#01_33#,
     KEY_LEFT_ALT      => 16#01_34#,
     KEY_RIGHT_META    => 16#01_35#,
     KEY_LEFT_META     => 16#01_36#,
     KEY_LEFT_SUPER    => 16#01_37#,
     KEY_RIGHT_SUPER   => 16#01_38#,
     KEY_MODE          => 16#01_39#,
     KEY_COMPOSE       => 16#01_3a#,
     KEY_HELP          => 16#01_3b#,
     KEY_PRINT         => 16#01_3c#,
     KEY_SYSREQ        => 16#01_3d#,
     KEY_BREAK         => 16#01_3e#,
     KEY_MENU          => 16#01_3f#,
     KEY_POWER         => 16#01_40#,
     KEY_EURO          => 16#01_41#,
     KEY_UNDO          => 16#01_42#,
     KEY_GRAVE         => 16#01_43#,
     KEY_KP_CLEAR      => 16#01_44#,
     KEY_COMMAND       => 16#01_45#,
     KEY_FUNCTION      => 16#01_46#,
     KEY_VOLUME_UP     => 16#01_47#,
     KEY_VOLUME_DOWN   => 16#01_48#,
     KEY_VOLUME_MUTE   => 16#01_49#,
     KEY_F16           => 16#01_4a#,
     KEY_F17           => 16#01_4b#,
     KEY_F18           => 16#01_4c#,
     KEY_F19           => 16#01_4d#,
     KEY_F20           => 16#01_4e#,
     KEY_F21           => 16#01_4f#,
     KEY_F22           => 16#01_50#,
     KEY_F23           => 16#01_51#,
     KEY_F24           => 16#01_52#,
     KEY_F25           => 16#01_53#,
     KEY_F26           => 16#01_54#,
     KEY_F27           => 16#01_55#,
     KEY_F28           => 16#01_56#,
     KEY_F29           => 16#01_57#,
     KEY_F30           => 16#01_58#,
     KEY_F31           => 16#01_59#,
     KEY_F32           => 16#01_5a#,
     KEY_F33           => 16#01_5b#,
     KEY_F34           => 16#01_5c#,
     KEY_F35           => 16#01_5d#,
     KEY_BEGIN         => 16#01_5e#,
     KEY_RESET         => 16#01_5f#,
     KEY_STOP          => 16#01_60#,
     KEY_USER          => 16#01_61#,
     KEY_SYSTEM        => 16#01_62#,
     KEY_PRINT_SCREEN  => 16#01_63#,
     KEY_CLEAR_LINE    => 16#01_64#,
     KEY_CLEAR_DISPLAY => 16#01_65#,
     KEY_INSERT_LINE   => 16#01_66#,
     KEY_DELETE_LINE   => 16#01_67#,
     KEY_INSERT_CHAR   => 16#01_68#,
     KEY_DELETE_CHAR   => 16#01_69#,
     KEY_PREV          => 16#01_6a#,
     KEY_NEXT          => 16#01_6b#,
     KEY_SELECT        => 16#01_6c#,
     KEY_EXECUTE       => 16#01_6d#,
     KEY_REDO          => 16#01_6e#,
     KEY_FIND          => 16#01_6f#,
     KEY_MODE_SWITCH   => 16#01_70#,
     KEY_LAST          => 16#01_71#,
     KEY_ANY           => 16#ff_ff#);
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
