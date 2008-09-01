with SDL.keysym;
with agar.core.timeout;
with agar.core.types;

package agar.gui.widget.editable is
  use type c.unsigned;

  type encoding_t is (ENCODING_UTF8, ENCODING_ASCII);
   for encoding_t use (ENCODING_UTF8 => 0, ENCODING_ASCII => 1);
   for encoding_t'size use c.unsigned'size;
  pragma convention (c, encoding_t);

  subtype flags_t is c.unsigned;
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
  EDITABLE_NO_HFILL      : constant flags_t := 16#00400#;
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

  type editable_t is record
    widget         : widget_t;
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
    repeat_key     : SDL.keysym.key_t;
    repeat_mod     : SDL.keysym.modkey_t;
    repeat_unicode : agar.core.types.uint32_t;
    ucs_buffer     : access agar.core.types.uint32_t;
    ucs_length     : c.unsigned;
  end record;
  type editable_access_t is access all editable_t;
  pragma convention (c, editable_t);
  pragma convention (c, editable_access_t);

end agar.gui.widget.editable;
