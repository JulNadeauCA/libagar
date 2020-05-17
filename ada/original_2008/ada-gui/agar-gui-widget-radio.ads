package agar.gui.widget.radio is

  use type c.unsigned;

  type flags_t is new c.unsigned;
  RADIO_HFILL  : constant flags_t := 16#01#;
  RADIO_VFILL  : constant flags_t := 16#02#;
  RADIO_EXPAND : constant flags_t := RADIO_HFILL or RADIO_VFILL;

  type radio_t is limited private;
  type radio_access_t is access all radio_t;
  pragma convention (c, radio_access_t);

  type item_t is limited private;
  type item_access_t is access all item_t;
  pragma convention (c, item_access_t);

  type item_text_t is new string;
  type item_text_access_t is access constant item_text_t;
  type item_text_array_t is array (positive range <>) of item_text_access_t;

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return radio_access_t;
  pragma inline (allocate);

  function add_item
    (radio : radio_access_t;
     text  : string) return boolean;
  pragma inline (add_item);

  function add_item_hotkey
    (radio : radio_access_t;
     key   : c.int;
     text  : string) return boolean;
  pragma inline (add_item_hotkey);

  procedure clear_items (radio : radio_access_t);
  pragma import (c, clear_items, "AG_RadioClearItems");

  function widget (radio : radio_access_t) return widget_access_t;
  pragma inline (widget);

private

  type text_t is array (1 .. 128) of aliased c.char;
  pragma convention (c, text_t);

  type item_t is record
    text    : text_t;
    surface : c.int;
    hotkey  : c.int;
  end record;
  pragma convention (c, item_t);

  type radio_t is record
    widget    : aliased widget_t;
    flags     : flags_t;
    value     : c.int;
    items     : item_access_t;
    nitems    : c.int;
    sel_item  : c.int;
    max_w     : c.int;
    oversel   : c.int;
    x_padding : c.int;
    y_padding : c.int;
    x_spacing : c.int;
    y_spacing : c.int;
    radius    : c.int;
    r         : agar.gui.rect.rect_t;
  end record;
  pragma convention (c, radio_t);

end agar.gui.widget.radio;
