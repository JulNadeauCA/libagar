with agar.core.types;
with agar.gui.surface;
with agar.gui.types;

package agar.gui.widget.icon is

  use type c.unsigned;

  subtype flags_t is agar.gui.types.widget_icon_flags_t;
  ICON_REGEN_LABEL : constant flags_t := 16#01#;
  ICON_DND         : constant flags_t := 16#02#;
  ICON_DBLCLICKED  : constant flags_t := 16#04#;
  ICON_BGFILL      : constant flags_t := 16#08#;

  subtype icon_t is agar.gui.types.widget_icon_t;
  subtype icon_access_t is agar.gui.types.widget_icon_access_t;

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return icon_access_t;
  pragma import (c, allocate, "AG_IconNew");

  function allocate_from_surface
    (surface : agar.gui.surface.surface_access_t) return icon_access_t;
  pragma import (c, allocate_from_surface, "AG_IconFromSurface");

  function allocate_from_bitmap
    (filename : string) return icon_access_t;
  pragma inline (allocate_from_bitmap);

--  procedure set_padding
--    (icon   : icon_access_t;
--     left   : natural;
--     right  : natural;
--     top    : natural;
--     bottom : natural);
--  pragma inline (set_padding);

  procedure set_surface
    (icon    : icon_access_t;
     surface : agar.gui.surface.surface_access_t);
  pragma import (c, set_surface, "AG_IconSetSurface");

  procedure set_surface_no_copy
    (icon    : icon_access_t;
     surface : agar.gui.surface.surface_access_t);
  pragma import (c, set_surface_no_copy, "AG_IconSetSurfaceNODUP");

  procedure set_text
    (icon : icon_access_t;
     text : string);
  pragma inline (set_text);

  procedure set_background_fill
    (icon  : icon_access_t;
     fill  : boolean;
     color : agar.core.types.uint32_t);
  pragma inline (set_background_fill);

  function widget (icon : icon_access_t) return widget_access_t renames
    agar.gui.types.widget_icon_widget;

end agar.gui.widget.icon;
