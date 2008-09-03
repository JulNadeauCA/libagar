with agar.core.timeout;

package agar.gui.widget.slider is

  use type c.unsigned;

  subtype flags_t is c.unsigned;
  SLIDER_HFILL     : constant flags_t := 16#01#;
  SLIDER_VFILL     : constant flags_t := 16#02#;
  SLIDER_FOCUSABLE : constant flags_t := 16#04#;
  SLIDER_EXPAND    : constant flags_t := SLIDER_HFILL or SLIDER_VFILL;

  type type_t is (SLIDER_HORIZ, SLIDER_VERT);
  for type_t use (SLIDER_HORIZ => 0, SLIDER_VERT => 1);
  for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  type button_t is (
    SLIDER_BUTTON_NONE,
    SLIDER_BUTTON_DEC,
    SLIDER_BUTTON_INC,
    SLIDER_BUTTON_SCROLL
  );
  for button_t use (
    SLIDER_BUTTON_NONE   => 0,
    SLIDER_BUTTON_DEC    => 1,
    SLIDER_BUTTON_INC    => 2,
    SLIDER_BUTTON_SCROLL => 3
  );
  for button_t'size use c.unsigned'size;
  pragma convention (c, button_t);

  type slider_t is record
    widget      : widget_t;
    flags       : flags_t;
    value       : c.int;
    min         : c.int;
    max         : c.int;
    slider_type : type_t;
    ctl_pressed : c.int;
    w_control   : c.int;
    inc_to      : agar.core.timeout.timeout_t;
    dec_to      : agar.core.timeout.timeout_t;
    x_offset    : c.int;
    extent      : c.int;
    r_inc       : c.double;
    i_inc       : c.int;
  end record;
  type slider_access_t is access all slider_t;
  pragma convention (c, slider_t);
  pragma convention (c, slider_access_t);

end agar.gui.widget.slider;
