with agar.core.types;

package agar.gui.widget.progress_bar is

  use type c.unsigned;

  type type_t is (PROGRESS_BAR_HORIZ, PROGRESS_BAR_VERT);
   for type_t use (PROGRESS_BAR_HORIZ => 0, PROGRESS_BAR_VERT => 1);
   for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  subtype flags_t is c.unsigned;
  PROGRESS_BAR_HFILL    : constant flags_t := 16#01#;
  PROGRESS_BAR_VFILL    : constant flags_t := 16#02#;
  PROGRESS_BAR_SHOW_PCT : constant flags_t := 16#04#;
  PROGRESS_BAR_EXPAND   : constant flags_t := PROGRESS_BAR_HFILL or PROGRESS_BAR_VFILL;

  type progress_bar_t is record
    widget   : widget_t;
    flags    : flags_t;
    value    : c.int;
    min      : c.int;
    max      : c.int;
    bar_type : type_t;
    width    : c.int;
    pad      : c.int;
    cache    : agar.core.types.void_ptr_t; -- XXX: ag_text_cache *
  end record;
  type progress_bar_access_t is access all progress_bar_t;
  pragma convention (c, progress_bar_t);
  pragma convention (c, progress_bar_access_t);

end agar.gui.widget.progress_bar;
