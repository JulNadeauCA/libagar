package body agar.gui.widget.editable is

  package cbinds is
    procedure set_static
      (editable : editable_access_t;
       enable   : c.int);
    pragma import (c, set_static, "AG_EditableSetStatic");

    procedure set_password
      (editable : editable_access_t;
       enable   : c.int);
    pragma import (c, set_password, "AG_EditableSetPassword");
 
    procedure set_integer_only
      (editable : editable_access_t;
       enable   : c.int);
    pragma import (c, set_integer_only, "AG_EditableSetIntOnly");
 
    procedure set_float_only
      (editable : editable_access_t;
       enable   : c.int);
    pragma import (c, set_float_only, "AG_EditableSetFltOnly");

    procedure size_hint
      (editable : editable_access_t;
       text     : cs.chars_ptr);
    pragma import (c, size_hint, "AG_EditableSizeHint");

    procedure size_hint_pixels
      (editable : editable_access_t;
       width    : c.int;
       height   : c.int);
    pragma import (c, size_hint_pixels, "AG_EditableSizeHintPixels");

    function map_position
      (editable : editable_access_t;
       x        : c.int;
       y        : c.int;
       pos      : access c.int;
       absolute : c.int) return c.int;
    pragma import (c, map_position, "AG_EditableMapPosition");

    procedure move_cursor
      (editable : editable_access_t;
       x        : c.int;
       y        : c.int;
       absolute : c.int);
    pragma import (c, move_cursor, "AG_EditableMoveCursor");

    function get_cursor_position (editable : editable_access_t) return c.int;
    pragma import (c, get_cursor_position, "AG_EditableGetCursorPos");

    function set_cursor_position
      (editable : editable_access_t;
       index    : c.int) return c.int;
    pragma import (c, set_cursor_position, "AG_EditableSetCursorPos");

    procedure set_string
      (editable : editable_access_t;
       text     : cs.chars_ptr);
    pragma import (c, set_string, "AG_EditableSetString");

    function get_integer (editable : editable_access_t) return c.int;
    pragma import (c, get_integer, "AG_EditableInt");

    function get_float (editable : editable_access_t) return c.c_float;
    pragma import (c, get_float, "AG_EditableFlt");

    function get_long_float (editable : editable_access_t) return c.double;
    pragma import (c, get_long_float, "AG_EditableDbl");
  end cbinds;

  procedure set_static
    (editable : editable_access_t;
     enable   : boolean) is
  begin
    if enable then
      cbinds.set_static (editable => editable, enable => 1);
    else
      cbinds.set_static (editable => editable, enable => 0);
    end if;
  end set_static;

  procedure set_password
    (editable : editable_access_t;
     enable   : boolean) is
  begin
    if enable then
      cbinds.set_password (editable => editable, enable => 1);
    else
      cbinds.set_password (editable => editable, enable => 0);
    end if;
  end set_password;

  procedure set_integer_only
    (editable : editable_access_t;
     enable   : boolean) is
  begin
    if enable then
      cbinds.set_integer_only (editable => editable, enable => 1);
    else
      cbinds.set_integer_only (editable => editable, enable => 0);
    end if;
  end set_integer_only;

  procedure set_float_only
    (editable : editable_access_t;
     enable   : boolean) is
  begin
    if enable then
      cbinds.set_float_only (editable => editable, enable => 1);
    else
      cbinds.set_float_only (editable => editable, enable => 0);
    end if;
  end set_float_only;

  procedure size_hint
    (editable : editable_access_t;
     text     : string)
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.size_hint
      (editable => editable,
       text     => cs.to_chars_ptr (ca_text'unchecked_access));
  end size_hint;

  procedure size_hint_pixels
    (editable : editable_access_t;
     width    : positive;
     height   : positive) is
  begin
    cbinds.size_hint_pixels
      (editable => editable,
       width    => c.int (width),
       height   => c.int (height));
  end size_hint_pixels;

  -- cursor manipulation

  procedure map_position
    (editable : editable_access_t;
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
      (editable => editable,
       x        => c.int (x),
       y        => c.int (y),
       pos      => c_pos'unchecked_access,
       absolute => c_abs);
    index := natural (c_ind);
    pos   := cursor_pos_t'val (c_pos);
  end map_position;

  procedure move_cursor
    (editable : editable_access_t;
     x        : integer;
     y        : integer;
     absolute : boolean)
  is
    c_abs : c.int := 0;
  begin
    if absolute then c_abs := 1; end if;
    cbinds.move_cursor
      (editable => editable,
       x        => c.int (x),
       y        => c.int (y),
       absolute => c_abs);
  end move_cursor;

  function get_cursor_position (editable : editable_access_t) return natural is
  begin
    return natural (cbinds.get_cursor_position (editable));
  end get_cursor_position;

  function set_cursor_position
    (editable : editable_access_t;
     index    : integer) return integer is
  begin
    return integer (cbinds.set_cursor_position (editable, c.int (index)));
  end set_cursor_position;

  -- text manipulation

  procedure set_string
    (editable : editable_access_t;
     text     : string)
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.set_string (editable, cs.to_chars_ptr (ca_text'unchecked_access));
  end set_string;

  function get_integer (editable : editable_access_t) return integer is
  begin
    return integer (cbinds.get_integer (editable));
  end get_integer;

  function get_float (editable : editable_access_t) return float is
  begin
    return float (cbinds.get_float (editable));
  end get_float;

  function get_long_float (editable : editable_access_t) return long_float is
  begin
    return long_float (cbinds.get_long_float (editable));
  end get_long_float;

  function widget (editable : editable_access_t) return widget_access_t is
  begin
    return editable.widget'access;
  end widget;

end agar.gui.widget.editable;
