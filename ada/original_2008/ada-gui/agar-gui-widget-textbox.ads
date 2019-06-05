with agar.gui.widget.scrollbar;
with agar.gui.widget.editable;

package agar.gui.widget.textbox is

  use type c.unsigned;

  subtype cursor_pos_t is agar.gui.widget.editable.cursor_pos_t;

  type flags_t is new c.unsigned;
  TEXTBOX_MULTILINE     : constant flags_t := 16#00001#;
  TEXTBOX_PASSWORD      : constant flags_t := 16#00004#;
  TEXTBOX_ABANDON_FOCUS : constant flags_t := 16#00008#;
  TEXTBOX_COMBO         : constant flags_t := 16#00010#;
  TEXTBOX_HFILL         : constant flags_t := 16#00020#;
  TEXTBOX_VFILL         : constant flags_t := 16#00040#;
  TEXTBOX_EXPAND        : constant flags_t := TEXTBOX_HFILL or TEXTBOX_VFILL;
  TEXTBOX_READONLY      : constant flags_t := 16#00100#;
  TEXTBOX_INT_ONLY      : constant flags_t := 16#00200#;
  TEXTBOX_FLT_ONLY      : constant flags_t := 16#00400#;
  TEXTBOX_CATCH_TAB     : constant flags_t := 16#00800#;
  TEXTBOX_CURSOR_MOVING : constant flags_t := 16#01000#;
  TEXTBOX_STATIC        : constant flags_t := 16#04000#;
  TEXTBOX_NOEMACS       : constant flags_t := 16#08000#;
  TEXTBOX_NOWORDSEEK    : constant flags_t := 16#10000#;
  TEXTBOX_NOLATIN1      : constant flags_t := 16#20000#;

  type textbox_t is limited private;
  type textbox_access_t is access all textbox_t;
  pragma convention (c, textbox_access_t);

  string_max : constant := agar.gui.widget.editable.string_max;

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string) return textbox_access_t;
  pragma inline (allocate);

  procedure set_static
    (textbox : textbox_access_t;
     enable  : boolean);
  pragma inline (set_static);

  procedure set_password
    (textbox : textbox_access_t;
     enable  : boolean);
  pragma inline (set_password);

  procedure set_float_only
    (textbox : textbox_access_t;
     enable  : boolean);
  pragma inline (set_float_only);

  procedure set_integer_only
    (textbox : textbox_access_t;
     enable  : boolean);
  pragma inline (set_integer_only);

  procedure set_label
    (textbox : textbox_access_t;
     text    : string);
  pragma inline (set_label);

  procedure size_hint
    (textbox : textbox_access_t;
     text    : string);
  pragma inline (size_hint);

  procedure size_hint_pixels
    (textbox : textbox_access_t;
     width   : positive;
     height  : positive);
  pragma inline (size_hint_pixels);

  -- cursor manipulation

  procedure map_position
    (textbox  : textbox_access_t;
     x        : integer;
     y        : integer;
     index    : out natural;
     pos      : out cursor_pos_t;
     absolute : boolean);
  pragma inline (map_position);

  function move_cursor
    (textbox  : textbox_access_t;
     x        : integer;
     y        : integer;
     absolute : boolean) return integer;
  pragma inline (move_cursor);

  function get_cursor_position (textbox : textbox_access_t) return integer;
  pragma inline (get_cursor_position);

  function set_cursor_position
    (textbox  : textbox_access_t;
     position : integer) return integer;
  pragma inline (set_cursor_position);

  -- text manipulation

  procedure set_string
    (textbox : textbox_access_t;
     text    : string);
  pragma inline (set_string);

  procedure set_string_ucs4
    (textbox : textbox_access_t;
     text    : wide_wide_string);
  pragma inline (set_string_ucs4);

  procedure clear_string (textbox : textbox_access_t);
  pragma import (c, clear_string, "agar_gui_widget_textbox_clear_string");

  procedure buffer_changed (textbox : textbox_access_t);
  pragma import (c, buffer_changed, "agar_gui_widget_textbox_buffer_changed");

  function get_integer (textbox : textbox_access_t) return integer;
  pragma inline (get_integer);

  function get_float (textbox : textbox_access_t) return float;
  pragma inline (get_float);

  function get_long_float (textbox : textbox_access_t) return long_float;
  pragma inline (get_long_float);

  function widget (textbox : textbox_access_t) return widget_access_t;
  pragma inline (widget);

private

  type textbox_t is record
    widget          : aliased widget_t;
    editable        : agar.gui.widget.editable.editable_access_t;

    label_text      : cs.chars_ptr;
    label           : c.int;

    flags           : flags_t;
    box_pad_x       : c.int;
    box_pad_y       : c.int;
    label_pad_left  : c.int;
    label_pad_right : c.int;
    width_label     : c.int;
    height_label    : c.int;
    horiz_scrollbar : agar.gui.widget.scrollbar.scrollbar_access_t;
    vert_scrollbar  : agar.gui.widget.scrollbar.scrollbar_access_t;

    r               : agar.gui.rect.rect_t;
    r_label         : agar.gui.rect.rect_t;
  end record;
  pragma convention (c, textbox_t);

end agar.gui.widget.textbox;
