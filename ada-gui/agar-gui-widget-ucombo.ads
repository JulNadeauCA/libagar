with agar.gui.widget.button;
with agar.gui.widget.tlist;
with agar.gui.window;

package agar.gui.widget.ucombo is

  use type c.unsigned;

  type flags_t is new c.unsigned;
  UCOMBO_HFILL  : constant flags_t := 16#01#;
  UCOMBO_VFILL  : constant flags_t := 16#02#;
  UCOMBO_EXPAND : constant flags_t := UCOMBO_HFILL or UCOMBO_VFILL;

  type ucombo_t is record
    widget     : widget_t;
    flags      : flags_t;
    button     : agar.gui.widget.button.button_access_t;
    list       : agar.gui.widget.tlist.tlist_access_t;
    panel      : agar.gui.window.window_access_t;
    w_saved    : c.int;
    h_saved    : c.int;
    w_pre_list : c.int;
    h_pre_list : c.int;
  end record;
  type ucombo_access_t is access all ucombo_t;
  pragma convention (c, ucombo_t);
  pragma convention (c, ucombo_access_t);

end agar.gui.widget.ucombo;
