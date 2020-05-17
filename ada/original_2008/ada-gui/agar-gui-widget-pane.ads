with agar.gui.widget.box;

package agar.gui.widget.pane is

  use type c.unsigned;

  type type_t is (PANE_HORIZ, PANE_VERT);
   for type_t use (PANE_HORIZ => 0, PANE_VERT => 1);
   for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  type pane_t is limited private;
  type pane_access_t is access all pane_t;
  pragma convention (c, pane_access_t);

  type flags_t is new c.unsigned;
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

  type partition_t is (NONE, FIRST, SECOND);
   for partition_t use (NONE => -1, FIRST => 0, SECOND => 1);
   for partition_t'size use c.int'size;
  pragma convention (c, partition_t);

  -- API

  function allocate
    (parent    : widget_access_t;
     pane_type : type_t;
     flags     : flags_t) return pane_access_t;
  pragma import (c, allocate, "AG_PaneNew");

  function allocate_horizontal
    (parent : widget_access_t;
     flags  : flags_t) return pane_access_t;
  pragma import (c, allocate_horizontal, "AG_PaneNewHoriz");

  function allocate_vertical
    (parent : widget_access_t;
     flags  : flags_t) return pane_access_t;
  pragma import (c, allocate_vertical, "AG_PaneNewVert");

  procedure attach_box
    (pane   : pane_access_t;
     which  : partition_t;
     box    : agar.gui.widget.box.box_access_t);
  pragma import (c, attach_box, "AG_PaneAttachBox");

  procedure attach_boxes
    (pane : pane_access_t;
     box1 : agar.gui.widget.box.box_access_t;
     box2 : agar.gui.widget.box.box_access_t);
  pragma import (c, attach_boxes, "AG_PaneAttachBoxes");

  procedure set_divider_width
    (pane   : pane_access_t;
     pixels : positive);
  pragma inline (set_divider_width);

  procedure set_division_minimum
    (pane      : pane_access_t;
     which     : partition_t;
     min_width : positive;
     max_width : positive);
  pragma inline (set_division_minimum);

  function move_divider
    (pane : pane_access_t;
     x    : positive) return positive;
  pragma inline (move_divider);

  function widget (pane : pane_access_t) return widget_access_t;
  pragma inline (widget);

private

  type pane_div_t is array (1 .. 2) of aliased agar.gui.widget.box.box_access_t;
  pragma convention (c, pane_div_t);
  type pane_geom_t is array (1 .. 2) of aliased c.int;
  pragma convention (c, pane_geom_t);

  type pane_t is record
    widget    : aliased widget_t;
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
  pragma convention (c, pane_t);

end agar.gui.widget.pane;
