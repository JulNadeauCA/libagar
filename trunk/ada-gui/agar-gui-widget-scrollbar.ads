with agar.core.event;
with agar.core.timeout;

package agar.gui.widget.scrollbar is

  use type c.unsigned;

  type type_t is (SCROLLBAR_HORIZ, SCROLLBAR_VERT);
   for type_t use (SCROLLBAR_HORIZ => 0, SCROLLBAR_VERT => 1);
   for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  type button_t is (
    SCROLLBAR_BUTTON_NONE,
    SCROLLBAR_BUTTON_DEC,
    SCROLLBAR_BUTTON_INC,
    SCROLLBAR_BUTTON_SCROLL
  );
  for button_t use (
    SCROLLBAR_BUTTON_NONE   => 0,
    SCROLLBAR_BUTTON_DEC    => 1,
    SCROLLBAR_BUTTON_INC    => 2,
    SCROLLBAR_BUTTON_SCROLL => 3
  );
  for button_t'size use c.unsigned'size;
  pragma convention (c, button_t);

  subtype flags_t is c.unsigned;
  SCROLLBAR_HFILL     : constant flags_t := 16#01#;
  SCROLLBAR_VFILL     : constant flags_t := 16#02#;
  SCROLLBAR_FOCUSABLE : constant flags_t := 16#04#;
  SCROLLBAR_EXPAND    : constant flags_t := SCROLLBAR_HFILL or SCROLLBAR_VFILL;

  type scrollbar_t is record
    widget          : widget_t;
    flags           : flags_t;
    value           : c.int;
    min             : c.int;
    max             : c.int;
    visible         : c.int;
    bar_type        : type_t;
    bar_button      : button_t;
    button_width    : c.int;
    button_def      : c.int;
    bar_width       : c.int;
    arrow_height    : c.int;
    button_inc_func : access agar.core.event.event_t;
    button_dec_func : access agar.core.event.event_t;
    scroll_to       : agar.core.timeout.timeout_t;
    inc_to          : agar.core.timeout.timeout_t;
    dec_to          : agar.core.timeout.timeout_t;
    x_offset        : c.int;
    extent          : c.int;
    r_inc           : c.double;
    i_inc           : c.int;
  end record;
  type scrollbar_access_t is access all scrollbar_t;
  pragma convention (c, scrollbar_t);
  pragma convention (c, scrollbar_access_t);

end agar.gui.widget.scrollbar;
