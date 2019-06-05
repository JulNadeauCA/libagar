with agar.core.types;
with agar.core.tail_queue;
with agar.gui.widget.label;

package agar.gui.widget.fixed_plotter is

  use type c.unsigned;

  type item_t is limited private;
  type item_access_t is access all item_t;
  pragma convention (c, item_access_t);

  type plotter_t;
  type plotter_access_t is access all plotter_t;
  pragma convention (c, plotter_access_t);

  package item_tail_queue is new agar.core.tail_queue
    (entry_type => item_access_t);

  subtype value_t is agar.core.types.uint16_t;

  type name_t is array (1 .. agar.gui.widget.label.max) of aliased c.char;
  pragma convention (c, name_t);

  type type_t is (PLOTTER_POINTS, PLOTTER_LINES);
   for type_t use (PLOTTER_POINTS => 0, PLOTTER_LINES => 1);
   for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  type flags_t is new c.unsigned;
  PLOTTER_SCROLL : constant flags_t := 16#01#;
  PLOTTER_XAXIS  : constant flags_t := 16#02#;
  PLOTTER_HFILL  : constant flags_t := 16#04#;
  PLOTTER_VFILL  : constant flags_t := 16#08#;
  PLOTTER_EXPAND : constant flags_t := PLOTTER_HFILL or PLOTTER_VFILL;

  type plotter_priv_t is limited private;
  type plotter_t is record
    widget       : aliased widget_t;
    plotter_type : type_t;
    privdata     : plotter_priv_t;
  end record;
  pragma convention (c, plotter_t);

  -- API

  function allocate
    (parent    : widget_access_t;
     plot_type : type_t;
     flags     : flags_t) return plotter_access_t;
  pragma import (c, allocate, "AG_FixedPlotterNew");

  function curve
    (plotter : plotter_access_t;
     name    : string;
     r       : agar.core.types.uint8_t;
     g       : agar.core.types.uint8_t;
     b       : agar.core.types.uint8_t;
     limit   : agar.core.types.uint32_t) return item_access_t;
  pragma inline (curve);

  procedure datum
    (plotter : plotter_access_t;
     value   : value_t);
  pragma import (c, datum, "AG_FixedPlotterDatum");

  function widget (plotter : plotter_access_t) return widget_access_t;
  pragma inline (widget);

private

  type plotter_priv_t is record
    flags        : flags_t;
    y_range      : value_t;
    x_offset     : value_t;
    y_origin     : c.int;
    items        : item_tail_queue.head_t;
  end record;
  pragma convention (c, plotter_priv_t);

  type item_t is record
    name      : name_t;
    color     : agar.core.types.uint32_t;
    values    : access value_t;
    nvalues   : agar.core.types.uint32_t;
    maxvalues : agar.core.types.uint32_t;
    limit     : agar.core.types.uint32_t;
    fpl       : plotter_access_t;
    items     : item_tail_queue.entry_t;
  end record;
  pragma convention (c, item_t);

end agar.gui.widget.fixed_plotter;
