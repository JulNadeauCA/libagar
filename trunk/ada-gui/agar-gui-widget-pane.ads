with agar.gui.widget.box;

package agar.gui.widget.pane is

  use type c.unsigned;

  type type_t is (PANE_HORIZ, PANE_VERT);
   for type_t use (PANE_HORIZ => 0, PANE_VERT => 1);
   for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  subtype flags_t is c.unsigned;
  PANE_HFILL          : constant flags_t := 16#001#;
  PANE_VFILL          : constant flags_t := 16#002#;
  PANE_DIV1FILL       : constant flags_t := 16#004#;
  PANE_FRAME          : constant flags_t := 16#008#;
  PANE_FORCE_DIV1FILL : constant flags_t := 16#010#;
  PANE_FORCE_DIV2FILL : constant flags_t := 16#020#;
  PANE_DIV            : constant flags_t := 16#040#;
  PANE_FORCE_DIV      : constant flags_t := 16#080#;
  PANE_INITSCALE      : constant flags_t := 16#100#;
  PANE_EXPAND         : constant flags_t := PANE_HFILL or PANE_VFILL;

  type pane_div_t is array (1 .. 2) of aliased agar.gui.widget.box.box_access_t;
  pragma convention (c, pane_div_t);
  type pane_geom_t is array (1 .. 2) of aliased c.int;
  pragma convention (c, pane_geom_t);

  type pane_t is record
    widget    : widget_t;
    pane_type : type_t;
    flags     : flags_t;
    div       : pane_div_t;
    minw      : pane_geom_t;
    minh      : pane_geom_t;
    dmoving   : c.int;
    dx        : c.int;
    rx        : c.int;
    wdiv      : c.int;
  end record;
  type pane_access_t is access all pane_t;
  pragma convention (c, pane_t);
  pragma convention (c, pane_access_t);

end agar.gui.widget.pane;
