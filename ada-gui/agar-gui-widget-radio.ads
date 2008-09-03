with SDL.keysym;

package agar.gui.widget.radio is

  use type c.unsigned;

  type text_t is array (1 .. 128) of aliased c.char;
  pragma convention (c, text_t);

  type item_t is record
    text    : text_t;
    surface : c.int;
    hotkey  : SDL.keysym.key_t; 
  end record;
  type item_access_t is access all item_t;
  pragma convention (c, item_t);
  pragma convention (c, item_access_t);

  subtype flags_t is c.unsigned;
  RADIO_HFILL  : constant flags_t := 16#01#;
  RADIO_VFILL  : constant flags_t := 16#02#;
  RADIO_EXPAND : constant flags_t := RADIO_HFILL or RADIO_VFILL;

  type radio_t is record
    widget    : widget_t;
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
  end record;
  type radio_access_t is access all radio_t;
  pragma convention (c, radio_t);
  pragma convention (c, radio_access_t);

end agar.gui.widget.radio;
