with agar.gui.widget.box;

package agar.gui.widget.mpane is

  use type c.unsigned;

  type layout_t is (
    MPANE1,
    MPANE2V,
    MPANE2H,
    MPANE2L1R,
    MPANE1L2R,
    MPANE2T1B,
    MPANE1T2B,
    MPANE3L1R,
    MPANE1L3R,
    MPANE3T1B,
    MPANE1T3B,
    MPANE4
  );
  for layout_t use (
    MPANE1    => 0,
    MPANE2V   => 1,
    MPANE2H   => 2,
    MPANE2L1R => 3,
    MPANE1L2R => 4,
    MPANE2T1B => 5,
    MPANE1T2B => 6,
    MPANE3L1R => 7,
    MPANE1L3R => 8,
    MPANE3T1B => 9,
    MPANE1T3B => 10,
    MPANE4    => 11
  );
  for layout_t'size use c.unsigned'size;
  pragma convention (c, layout_t);

  type flags_t is new c.unsigned;
  MPANE_HFILL     : constant flags_t := 16#01#;
  MPANE_VFILL     : constant flags_t := 16#02#;
  MPANE_FRAMES    : constant flags_t := 16#04#;
  MPANE_FORCE_DIV : constant flags_t := 16#08#;
  MPANE_EXPAND    : constant flags_t := MPANE_HFILL or MPANE_VFILL;

  type panes_t is array (1 .. 4) of aliased agar.gui.widget.box.box_access_t;
  pragma convention (c, panes_t);

  type mpane_t is record
    box    : aliased agar.gui.widget.box.box_t;
    layout : layout_t;
    flags  : flags_t;
    panes  : panes_t;
    npanes : c.unsigned;
  end record;
  type mpane_access_t is access all mpane_t;
  pragma convention (c, mpane_t);
  pragma convention (c, mpane_access_t);

  -- API

  function allocate
    (parent : widget_access_t;
     layout : layout_t;
     flags  : flags_t) return mpane_access_t;
  pragma import (c, allocate, "AG_MPaneNew");

  procedure set_layout
    (mpane  : mpane_access_t;
     layout : layout_t);
  pragma import (c, set_layout, "AG_MPaneSetLayout");

  function widget (mpane : mpane_access_t) return widget_access_t;
  pragma inline (widget);

end agar.gui.widget.mpane;
