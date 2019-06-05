with agar.core.types;

package agar.gui.widget.progress_bar is

  use type c.unsigned;

  type type_t is (PROGRESS_BAR_HORIZ, PROGRESS_BAR_VERT);
   for type_t use (PROGRESS_BAR_HORIZ => 0, PROGRESS_BAR_VERT => 1);
   for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  type flags_t is new c.unsigned;
  PROGRESS_BAR_HFILL    : constant flags_t := 16#01#;
  PROGRESS_BAR_VFILL    : constant flags_t := 16#02#;
  PROGRESS_BAR_SHOW_PCT : constant flags_t := 16#04#;
  PROGRESS_BAR_EXPAND   : constant flags_t := PROGRESS_BAR_HFILL or PROGRESS_BAR_VFILL;

  type progress_bar_t is limited private;
  type progress_bar_access_t is access all progress_bar_t;
  pragma convention (c, progress_bar_access_t);

  type percent_t is new c.int range 0 .. 100;
  type percent_access_t is access all percent_t;
  pragma convention (c, percent_t);
  pragma convention (c, percent_access_t);

  -- API

  function allocate
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t) return progress_bar_access_t;
  pragma import (c, allocate, "AG_ProgressBarNew");

  function allocate_horizontal
    (parent : widget_access_t;
     flags  : flags_t) return progress_bar_access_t;
  pragma import (c, allocate_horizontal, "AG_ProgressBarNewHoriz");

  function allocate_vertical
    (parent : widget_access_t;
     flags  : flags_t) return progress_bar_access_t;
  pragma import (c, allocate_vertical, "AG_ProgressBarNewVert");

  procedure set_width
    (bar   : progress_bar_access_t;
     width : natural);
  pragma inline (set_width);

  function percent (bar : progress_bar_access_t) return percent_t;
  pragma import (c, percent, "AG_ProgressBarPercent");

  function widget (bar : progress_bar_access_t) return widget_access_t;
  pragma inline (widget);

private

  type progress_bar_t is record
    widget   : aliased widget_t;
    flags    : flags_t;
    value    : c.int;
    min      : c.int;
    max      : c.int;
    bar_type : type_t;
    width    : c.int;
    pad      : c.int;
    cache    : agar.core.types.void_ptr_t; -- XXX: ag_text_cache *
  end record;
  pragma convention (c, progress_bar_t);

end agar.gui.widget.progress_bar;
