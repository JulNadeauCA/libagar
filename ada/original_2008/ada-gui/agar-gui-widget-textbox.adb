package body agar.gui.widget.textbox is

  package cbinds is
    function allocate
      (parent : widget_access_t;
       flags  : flags_t;
       label  : cs.chars_ptr) return textbox_access_t;
    pragma import (c, allocate, "AG_TextboxNewS");

    procedure set_static
      (textbox : textbox_access_t;
       enable  : c.int);
    pragma import (c, set_static, "agar_gui_widget_textbox_set_static");

    procedure set_password
      (textbox : textbox_access_t;
       enable  : c.int);
    pragma import (c, set_password, "agar_gui_widget_textbox_set_password");

    procedure set_float_only
      (textbox : textbox_access_t;
       enable  : c.int);
    pragma import (c, set_float_only, "agar_gui_widget_textbox_set_float_only");

    procedure set_integer_only
      (textbox : textbox_access_t;
       enable  : c.int);
    pragma import (c, set_integer_only, "agar_gui_widget_textbox_set_integer_only");

    procedure set_label
      (textbox : textbox_access_t;
       text    : cs.chars_ptr);
    pragma import (c, set_label, "AG_TextboxSetLabelS");

    procedure size_hint
      (textbox : textbox_access_t;
       text    : cs.chars_ptr);
    pragma import (c, size_hint, "agar_gui_widget_textbox_size_hint");

    procedure size_hint_pixels
      (textbox : textbox_access_t;
       width   : c.unsigned;
       height  : c.unsigned);
    pragma import (c, size_hint_pixels, "agar_gui_widget_textbox_size_hint_pixels");

    function map_position
      (textbox  : textbox_access_t;
       x        : c.int;
       y        : c.int;
       pos      : access c.int;
       absolute : c.int) return c.int;
    pragma import (c, map_position, "agar_gui_widget_textbox_map_position");

    function move_cursor
      (textbox  : textbox_access_t;
       x        : c.int;
       y        : c.int;
       absolute : c.int) return c.int;
    pragma import (c, move_cursor, "agar_gui_widget_textbox_move_cursor");

    function get_cursor_position (textbox : textbox_access_t) return c.int;
    pragma import (c, get_cursor_position, "agar_gui_widget_textbox_get_cursor_pos");

    function set_cursor_position
      (textbox  : textbox_access_t;
       position : c.int) return c.int;
    pragma import (c, set_cursor_position, "agar_gui_widget_textbox_set_cursor_pos");

    procedure set_string
      (textbox : textbox_access_t;
       text    : cs.chars_ptr);
    pragma import (c, set_string, "agar_gui_widget_textbox_set_string");

    procedure set_string_ucs4
      (textbox : textbox_access_t;
       text    : access c.char32_t);
    pragma import (c, set_string_ucs4, "agar_gui_widget_textbox_set_string_ucs4");

    function get_integer (textbox : textbox_access_t) return c.int;
    pragma import (c, get_integer, "agar_gui_widget_textbox_int");

    function get_float (textbox : textbox_access_t) return c.c_float;
    pragma import (c, get_float, "agar_gui_widget_textbox_float");

    function get_long_float (textbox : textbox_access_t) return c.double;
    pragma import (c, get_long_float, "agar_gui_widget_textbox_double");
  end cbinds;

  function allocate
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string) return textbox_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (c_label'unchecked_access));
  end allocate;

  procedure set_static
    (textbox : textbox_access_t;
     enable  : boolean) is
  begin
    if enable then
      cbinds.set_static (textbox, 1);
    else
      cbinds.set_static (textbox, 0);
    end if;
  end set_static;

  procedure set_password
    (textbox : textbox_access_t;
     enable  : boolean) is
  begin
    if enable then
      cbinds.set_password (textbox, 1);
    else
      cbinds.set_password (textbox, 0);
    end if;
  end set_password;

  procedure set_float_only
    (textbox : textbox_access_t;
     enable  : boolean) is
  begin
    if enable then
      cbinds.set_float_only (textbox, 1);
    else
      cbinds.set_float_only (textbox, 0);
    end if;
  end set_float_only;

  procedure set_integer_only
    (textbox : textbox_access_t;
     enable  : boolean) is
  begin
    if enable then
      cbinds.set_integer_only (textbox, 1);
    else
      cbinds.set_integer_only (textbox, 0);
    end if;
  end set_integer_only;

  procedure set_label
    (textbox : textbox_access_t;
     text    : string)
  is
    c_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.set_label
      (textbox => textbox,
       text    => cs.to_chars_ptr (c_text'unchecked_access));
  end set_label;

  procedure size_hint
    (textbox : textbox_access_t;
     text    : string) is
    c_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.size_hint
      (textbox => textbox,
       text    => cs.to_chars_ptr (c_text'unchecked_access));
  end size_hint;

  procedure size_hint_pixels
    (textbox : textbox_access_t;
     width   : positive;
     height  : positive) is
  begin
    cbinds.size_hint_pixels
      (textbox => textbox,
       width   => c.unsigned (width),
       height  => c.unsigned (height));
  end size_hint_pixels;

  procedure map_position
    (textbox  : textbox_access_t;
     x        : integer;
     y        : integer;
     index    : out natural;
     pos      : out cursor_pos_t;
     absolute : boolean)
  is
    c_abs : c.int := 0;
    c_pos : aliased c.int;
    c_ind : c.int;
  begin
    if absolute then c_abs := 1; end if;
    c_ind := cbinds.map_position
      (textbox  => textbox,
       x        => c.int (x),
       y        => c.int (y),
       pos      => c_pos'unchecked_access,
       absolute => c_abs);
    index := natural (c_ind);
    pos   := cursor_pos_t'val (c_pos);
  end map_position;

  function move_cursor
    (textbox  : textbox_access_t;
     x        : integer;
     y        : integer;
     absolute : boolean) return integer is
  begin
    if absolute then
      return integer (cbinds.move_cursor
        (textbox  => textbox,
         x        => c.int (x),
         y        => c.int (y),
         absolute => 1));
    else
      return integer (cbinds.move_cursor
        (textbox  => textbox,
         x        => c.int (x),
         y        => c.int (y),
         absolute => 0));
    end if;
  end move_cursor;

  function get_cursor_position (textbox : textbox_access_t) return integer is
  begin
    return integer (cbinds.get_cursor_position (textbox));
  end get_cursor_position;

  function set_cursor_position
    (textbox  : textbox_access_t;
     position : integer) return integer is
  begin
    return integer (cbinds.set_cursor_position
      (textbox  => textbox,
       position => c.int (position)));
  end set_cursor_position;

  -- text manipulation

  procedure set_string
    (textbox : textbox_access_t;
     text    : string)
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.set_string (textbox, cs.to_chars_ptr (ca_text'unchecked_access));
  end set_string;

  procedure set_string_ucs4
    (textbox : textbox_access_t;
     text    : wide_wide_string) is
    ca_text : aliased c.char32_array := c.to_c (text);
  begin
    cbinds.set_string_ucs4 (textbox, ca_text(ca_text'first)'unchecked_access);
  end set_string_ucs4;

  function get_integer (textbox : textbox_access_t) return integer is
  begin
    return integer (cbinds.get_integer (textbox));
  end get_integer;

  function get_float (textbox : textbox_access_t) return float is
  begin
    return float (cbinds.get_float (textbox));
  end get_float;

  function get_long_float (textbox : textbox_access_t) return long_float is
  begin
    return long_float (cbinds.get_long_float (textbox));
  end get_long_float;

  function widget (textbox : textbox_access_t) return widget_access_t is
  begin
    return textbox.widget'access;
  end widget;

end agar.gui.widget.textbox;
