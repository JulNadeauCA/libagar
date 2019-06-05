package agar.gui.widget.box is

  use type c.unsigned;

  type type_t is (BOX_HORIZ, BOX_VERT);
   for type_t use (BOX_HORIZ => 0, BOX_VERT => 1);
   for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  type flags_t is new c.unsigned;
  BOX_HOMOGENOUS : constant flags_t := 16#01#;
  BOX_HFILL      : constant flags_t := 16#02#;
  BOX_VFILL      : constant flags_t := 16#04#;
  BOX_FRAME      : constant flags_t := 16#08#;
  BOX_EXPAND     : constant flags_t := BOX_HFILL or BOX_VFILL;

  type box_t is limited private;
  type box_access_t is access all box_t;
  pragma convention (c, box_access_t);

  function allocate
    (parent   : widget_access_t;
     box_type : type_t;
     flags    : flags_t) return box_access_t;
  pragma import (c, allocate, "AG_BoxNew");

  function allocate_horizontal
    (parent   : widget_access_t;
     flags    : flags_t) return box_access_t;
  pragma inline (allocate_horizontal);
 
  function allocate_vertical
    (parent   : widget_access_t;
     flags    : flags_t) return box_access_t;
  pragma inline (allocate_vertical);
 
  procedure set_homogenous
    (box        : box_access_t;
     homogenous : boolean);
  pragma inline (set_homogenous);

  procedure set_padding
    (box     : box_access_t;
     padding : natural);
  pragma inline (set_padding);

  procedure set_spacing
    (box     : box_access_t;
     spacing : natural);
  pragma inline (set_spacing);

  procedure set_depth
    (box   : box_access_t;
     depth : natural);
  pragma inline (set_depth);

  function widget (box : box_access_t) return widget_access_t;
  pragma inline (widget);

private

  type box_t is record
    widget   : aliased widget_t;
    box_type : type_t;
    flags    : flags_t; 
    padding  : c.int;
    spacing  : c.int;
    depth    : c.int;
  end record;
  pragma convention (c, box_t);

end agar.gui.widget.box;
