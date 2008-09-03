with agar.gui.unit;
with agar.gui.widget.button;
with agar.gui.widget.textbox;
with agar.gui.widget.ucombo;

package agar.gui.widget.numerical is

  use type c.unsigned;

  subtype flags_t is c.unsigned;
  NUMERICAL_HFILL    : constant flags_t := 16#01#;
  NUMERICAL_VFILL    : constant flags_t := 16#02#;
  NUMERICAL_NO_HFILL : constant flags_t := 16#04#;

  type format_t is array (1 .. 32) of aliased c.char;
  pragma convention (c, format_t);

  type numerical_t is record
    widget     : widget_t;
    flags      : flags_t;
    value      : c.double;
    min        : c.double;
    max        : c.double;
    inc        : c.double;
    format     : format_t;
    unit       : agar.gui.unit.unit_access_t;
    writeable  : c.int;
    input      : agar.gui.widget.textbox.textbox_access_t;
    units      : agar.gui.widget.ucombo.ucombo_access_t;
    inc_bu     : agar.gui.widget.button.button_access_t;
    dec_bu     : agar.gui.widget.button.button_access_t;
    w_unit_sel : c.int;
    h_unit_sel : c.int;
    w_pre_unit : c.int;
  end record;
  type numerical_access_t is access all numerical_t;
  pragma convention (c, numerical_t);
  pragma convention (c, numerical_access_t);

end agar.gui.widget.numerical;
