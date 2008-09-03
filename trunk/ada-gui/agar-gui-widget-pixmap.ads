package agar.gui.widget.pixmap is

  use type c.unsigned;

  subtype flags_t is c.unsigned;
  PIXMAP_HFILL      : constant flags_t := 16#01#;
  PIXMAP_VFILL      : constant flags_t := 16#02#;
  PIXMAP_FORCE_SIZE : constant flags_t := 16#04#;
  PIXMAP_EXPAND     : constant flags_t := PIXMAP_HFILL or PIXMAP_VFILL;

  type pixmap_t is record
    widget : widget_t;
    flags  : flags_t;
    n      : c.int;
    s      : c.int;
    t      : c.int;
    pre_w  : c.int;
    pre_h  : c.int;
  end record;
  type pixmap_access_t is access all pixmap_t;
  pragma convention (c, pixmap_t);
  pragma convention (c, pixmap_access_t);

end agar.gui.widget.pixmap;
