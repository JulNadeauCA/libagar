with agar.core.timeout;
with agar.core.types;

package agar.gui.widget.editable is
  use type c.unsigned;

  type encoding_t is (ENCODING_UTF8, ENCODING_ASCII);
   for encoding_t use (ENCODING_UTF8 => 0, ENCODING_ASCII => 1);
   for encoding_t'size use c.unsigned'size;
  pragma convention (c, encoding_t);

  type flags_t is new c.unsigned;
  EDITABLE_HFILL         : constant flags_t := 16#00001#;
  EDITABLE_VFILL         : constant flags_t := 16#00002#;
  EDITABLE_EXPAND        : constant flags_t := EDITABLE_HFILL or EDITABLE_VFILL;
  EDITABLE_MULTILINE     : constant flags_t := 16#00004#;
  EDITABLE_BLINK_ON      : constant flags_t := 16#00008#;
  EDITABLE_PASSWORD      : constant flags_t := 16#00010#;
  EDITABLE_ABANDON_FOCUS : constant flags_t := 16#00020#;
  EDITABLE_INT_ONLY      : constant flags_t := 16#00040#;
  EDITABLE_FLT_ONLY      : constant flags_t := 16#00080#;
  EDITABLE_CATCH_TAB     : constant flags_t := 16#00100#;
  EDITABLE_CURSOR_MOVING : constant flags_t := 16#00200#;
  EDITABLE_NOSCROLL      : constant flags_t := 16#00800#;
  EDITABLE_NOSCROLL_ONCE : constant flags_t := 16#01000#;
  EDITABLE_MARKPREF      : constant flags_t := 16#02000#;
  EDITABLE_STATIC        : constant flags_t := 16#04000#;
  EDITABLE_NOEMACS       : constant flags_t := 16#08000#;
  EDITABLE_NOWORDSEEK    : constant flags_t := 16#10000#;
  EDITABLE_NOLATIN1      : constant flags_t := 16#20000#;

  string_max : constant := 1024;
  type string_t is array (1 .. string_max) of aliased c.char;
  pragma convention (c, string_t);

  type editable_t is limited private;
  type editable_access_t is access all editable_t;
  pragma convention (c, editable_access_t);

  type cursor_pos_t is (CURSOR_BEFORE, CURSOR_IN, CURSOR_AFTER);
   for cursor_pos_t use (CURSOR_BEFORE => -1, CURSOR_IN => 0, CURSOR_AFTER => 1);

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return editable_access_t;
  pragma import (c, allocate, "AG_EditableNew");

  -- missing: bind_utf8 - unsure of how to bind
  -- missing: bind_ascii

  procedure set_static
    (editable : editable_access_t;
     enable   : boolean);
  pragma inline (set_static);

  procedure set_password
    (editable : editable_access_t;
     enable   : boolean);
  pragma inline (set_password);

  procedure set_integer_only
    (editable : editable_access_t;
     enable   : boolean);
  pragma inline (set_integer_only);

  procedure set_float_only
    (editable : editable_access_t;
     enable   : boolean);
  pragma inline (set_float_only);

  procedure size_hint
    (editable : editable_access_t;
     text     : string);
  pragma inline (size_hint);

  procedure size_hint_pixels
    (editable : editable_access_t;
     width    : positive;
     height   : positive);
  pragma inline (size_hint_pixels);

  -- cursor manipulation

  procedure map_position
    (editable : editable_access_t;
     x        : integer;
     y        : integer;
     index    : out natural;
     pos      : out cursor_pos_t;
     absolute : boolean);
  pragma inline (map_position);

  procedure move_cursor
    (editable : editable_access_t;
     x        : integer;
     y        : integer;
     absolute : boolean);
  pragma inline (move_cursor);

  function get_cursor_position (editable : editable_access_t) return natural;
  pragma inline (get_cursor_position);

  function set_cursor_position
    (editable : editable_access_t;
     index    : integer) return integer;
  pragma inline (set_cursor_position);

  -- text manipulation

  procedure set_string
    (editable : editable_access_t;
     text     : string);
  pragma inline (set_string);

  procedure clear_string (editable : editable_access_t);
  pragma import (c, clear_string, "AG_EditableClearString");

  procedure buffer_changed (editable : editable_access_t);
  pragma import (c, buffer_changed, "AG_EditableBufferChanged");

  function get_integer (editable : editable_access_t) return integer;
  pragma inline (get_integer);

  function get_float (editable : editable_access_t) return float;
  pragma inline (get_float);

  function get_long_float (editable : editable_access_t) return long_float;
  pragma inline (get_long_float);

  function widget (editable : editable_access_t) return widget_access_t;
  pragma inline (widget);

private

  type editable_t is record
    widget         : aliased widget_t;

    flags          : flags_t;

    encoding       : encoding_t;
    str            : string_t;
    width_pre      : c.int;
    height_pre     : c.int;
    pos            : c.int;
    compose        : agar.core.types.uint32_t;
    x_cursor       : c.int;
    y_cursor       : c.int;
    x_cursor_pref  : c.int;

    x_sel1         : c.int;
    x_sel2         : c.int;
    sel_edit       : c.int;

    to_delay       : agar.core.timeout.timeout_t;
    to_repeat      : agar.core.timeout.timeout_t;
    to_blink       : agar.core.timeout.timeout_t;

    x              : c.int;
    x_max          : c.int;
    y              : c.int;
    y_max          : c.int;
    y_vis          : c.int;
    wheel_ticks    : agar.core.types.uint32_t;
    repeat_key     : c.int;
    repeat_mod     : c.int;
    repeat_unicode : agar.core.types.uint32_t;
    ucs_buffer     : access agar.core.types.uint32_t;
    ucs_length     : c.unsigned;
    r              : agar.gui.rect.rect_t;
  end record;
  pragma convention (c, editable_t);

end agar.gui.widget.editable;
