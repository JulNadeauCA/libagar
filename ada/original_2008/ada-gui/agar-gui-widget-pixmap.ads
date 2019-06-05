package agar.gui.widget.pixmap is

  use type c.unsigned;

  type flags_t is new c.unsigned;
  PIXMAP_HFILL      : constant flags_t := 16#01#;
  PIXMAP_VFILL      : constant flags_t := 16#02#;
  PIXMAP_FORCE_SIZE : constant flags_t := 16#04#;
  PIXMAP_EXPAND     : constant flags_t := PIXMAP_HFILL or PIXMAP_VFILL;

  type s_coord_t is new c.int;
  type t_coord_t is new c.int;
  pragma convention (c, s_coord_t);
  pragma convention (c, t_coord_t);

  type pixmap_t is limited private;
  type pixmap_access_t is access all pixmap_t;
  pragma convention (c, pixmap_access_t);

  function allocate
    (parent : widget_access_t;
     flags  : flags_t;
     width  : positive;
     height : positive) return pixmap_access_t;
  pragma inline (allocate);

  function from_surface
    (parent : widget_access_t;
     flags  : flags_t;
     source : agar.gui.surface.surface_access_t) return pixmap_access_t;
  pragma import (c, from_surface, "AG_PixmapFromSurface");

  function from_surface_copy
    (parent : widget_access_t;
     flags  : flags_t;
     source : agar.gui.surface.surface_access_t) return pixmap_access_t;
  pragma import (c, from_surface_copy, "AG_PixmapFromSurfaceCopy");

  function from_surface_scaled
    (parent : widget_access_t;
     flags  : flags_t;
     source : agar.gui.surface.surface_access_t;
     width  : positive;
     height : positive) return pixmap_access_t;
  pragma inline (from_surface_scaled);

  function from_bitmap
    (parent : widget_access_t;
     path   : string) return pixmap_access_t;
  pragma inline (from_bitmap);

--  function from_xcf
--    (parent : widget_access_t;
--     path   : string) return pixmap_access_t;
--  pragma inline (from_xcf);

  -- changing surfaces

  function add_surface
    (pixmap  : pixmap_access_t;
     surface : agar.gui.surface.surface_access_t) return boolean;
  pragma inline (add_surface);

  function add_surface_copy
    (pixmap  : pixmap_access_t;
     surface : agar.gui.surface.surface_access_t) return boolean;
  pragma inline (add_surface_copy);

  function add_surface_scaled
    (pixmap  : pixmap_access_t;
     surface : agar.gui.surface.surface_access_t;
     width   : positive;
     height  : positive) return boolean;
  pragma inline (add_surface_scaled);

  procedure set_surface
    (pixmap : pixmap_access_t;
     name   : agar.gui.surface.index_t);
  pragma import (c, set_surface, "agar_gui_widget_pixmap_set_surface");

  procedure replace_surface
    (pixmap : pixmap_access_t;
     name   : agar.gui.surface.index_t);
  pragma import (c, replace_surface, "agar_gui_widget_pixmap_replace_surface");

  procedure replace_surface_scaled
    (pixmap : pixmap_access_t;
     name   : agar.gui.surface.index_t;
     width  : positive;
     height : positive);
  pragma import (c, replace_surface_scaled, "AG_PixmapReplaceSurfaceScaled");

  procedure update_surface (pixmap : pixmap_access_t);
  pragma import (c, update_surface, "agar_gui_widget_pixmap_update_surface");

  procedure set_coordinates
    (pixmap : pixmap_access_t;
     s      : s_coord_t;
     t      : t_coord_t);
  pragma import (c, set_coordinates, "agar_gui_widget_pixmap_set_coords");

  function widget (pixmap : pixmap_access_t) return widget_access_t;
  pragma inline (widget);

private

  type pixmap_t is record
    widget : aliased widget_t;
    flags  : flags_t;

    n      : c.int;
    s      : c.int;
    t      : c.int;
    pre_w  : c.int;
    pre_h  : c.int;
    scaled : c.int;
    r      : agar.gui.rect.rect_t;
  end record;
  pragma convention (c, pixmap_t);

end agar.gui.widget.pixmap;
