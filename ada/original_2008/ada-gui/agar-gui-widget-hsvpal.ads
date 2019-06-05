with agar.core.types;
with agar.gui.rect;
with agar.gui.widget.menu;
with agar.gui.window;

package agar.gui.widget.hsvpal is

  use type c.unsigned;

  type flags_t is new c.unsigned;
  HSVPAL_PIXEL  : constant flags_t := 16#01#;
  HSVPAL_DIRTY  : constant flags_t := 16#02#;
  HSVPAL_HFILL  : constant flags_t := 16#04#;
  HSVPAL_VFILL  : constant flags_t := 16#08#;
  HSVPAL_EXPAND : constant flags_t := HSVPAL_HFILL or HSVPAL_VFILL;

  type circle_t is limited private;
  type triangle_t is limited private;
  type state_t is limited private;
  type hsvpal_t is limited private;

  type hsvpal_access_t is access all hsvpal_t;
  pragma convention (c, hsvpal_access_t);

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return hsvpal_access_t;
  pragma import (c, allocate, "AG_HSVPalNew");

  function widget (hsvpal : hsvpal_access_t) return widget_access_t;
  pragma inline (widget);

private

  type circle_t is record
    x       : c.int;
    y       : c.int;
    r_out   : c.int;
    r_in    : c.int;
    spacing : c.int;
    width   : c.int;
    dh      : c.c_float;
  end record;
  pragma convention (c, circle_t);

  type triangle_t is record
    x : c.int;
    y : c.int;
    w : c.int;
    h : c.int;
  end record;
  pragma convention (c, triangle_t);

  type state_t is (HSVPAL_SEL_NONE, HSVPAL_SEL_H, HSVPAL_SEL_SV, HSVPAL_SEL_A);
  for state_t use (
    HSVPAL_SEL_NONE => 0,
    HSVPAL_SEL_H    => 1,
    HSVPAL_SEL_SV   => 2,
    HSVPAL_SEL_A    => 3
  );
  for state_t'size use c.unsigned'size;
  pragma convention (c, state_t);

  type hsvpal_t is record
    widget       : aliased widget_t;
    flags        : flags_t;
    h            : c.c_float;
    s            : c.c_float;
    v            : c.c_float;
    a            : c.c_float;
    pixel        : agar.core.types.uint32_t;
    r_alpha      : agar.gui.rect.rect_t;
    surface      : agar.gui.surface.surface_access_t;
    sel_circle_r : c.int;
    circle       : circle_t;
    triangle     : triangle_t;
    state        : state_t;
    menu         : agar.gui.widget.menu.menu_access_t;
    menu_item    : agar.gui.widget.menu.item_access_t;
    menu_win     : agar.gui.window.window_access_t;
    c_tile       : agar.core.types.uint32_t;
  end record;
  pragma convention (c, hsvpal_t);

end agar.gui.widget.hsvpal;
