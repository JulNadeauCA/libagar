with agar.core.event;
with agar.gui.widget.button;
with agar.gui.widget.tlist;
with agar.gui.window;

package agar.gui.widget.ucombo is

  use type c.unsigned;

  type flags_t is new c.unsigned;
  UCOMBO_HFILL  : constant flags_t := 16#01#;
  UCOMBO_VFILL  : constant flags_t := 16#02#;
  UCOMBO_EXPAND : constant flags_t := UCOMBO_HFILL or UCOMBO_VFILL;

  type ucombo_t is limited private;
  type ucombo_access_t is access all ucombo_t;
  pragma convention (c, ucombo_access_t);

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return ucombo_access_t;
  pragma import (c, allocate, "AG_UComboNew");

  function allocate_polled
    (parent   : widget_access_t;
     flags    : flags_t;
     callback : agar.core.event.callback_t) return ucombo_access_t;
  pragma inline (allocate_polled);

  procedure size_hint
    (ucombo : ucombo_access_t;
     text   : string;
     items  : natural);
  pragma inline (size_hint);

  procedure size_hint_pixels
    (ucombo : ucombo_access_t;
     width  : natural;
     height : natural);
  pragma inline (size_hint_pixels);

  -- selection

  procedure combo_select
    (ucombo : ucombo_access_t;
     item   : agar.gui.widget.tlist.item_access_t);
  pragma import (c, combo_select, "AG_ComboSelect");

  --

  function widget (ucombo : ucombo_access_t) return widget_access_t;
  pragma inline (widget);

private

  type ucombo_t is record
    widget     : aliased widget_t;
    flags      : flags_t;
    button     : agar.gui.widget.button.button_access_t;
    list       : agar.gui.widget.tlist.tlist_access_t;
    panel      : agar.gui.window.window_access_t;
    w_saved    : c.int;
    h_saved    : c.int;
    w_pre_list : c.int;
    h_pre_list : c.int;
  end record;
  pragma convention (c, ucombo_t);

end agar.gui.widget.ucombo;
