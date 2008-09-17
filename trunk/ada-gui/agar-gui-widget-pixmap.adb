package body agar.gui.widget.pixmap is

  use type c.int;

  package cbinds is
    function allocate
      (parent : widget_access_t;
       flags  : flags_t;
       width  : c.int;
       height : c.int) return pixmap_access_t;
    pragma import (c, allocate, "AG_PixmapNew");

    function add_surface
      (pixmap  : pixmap_access_t;
       surface : agar.gui.surface.surface_access_t) return c.int;
    pragma import (c, add_surface, "AG_PixmapAddSurface");

    function add_surface_copy
      (pixmap  : pixmap_access_t;
       surface : agar.gui.surface.surface_access_t) return c.int;
    pragma import (c, add_surface_copy, "AG_PixmapAddSurfaceCopy");

    function add_surface_scaled
      (pixmap  : pixmap_access_t;
       surface : agar.gui.surface.surface_access_t;
       width   : c.int;
       height  : c.int) return c.int;
    pragma import (c, add_surface_scaled, "AG_PixmapAddSurfaceScaled");

    function from_surface_scaled
      (parent : widget_access_t;
       flags  : flags_t;
       source : agar.gui.surface.surface_access_t;
       width  : c.int;
       height : c.int) return pixmap_access_t;
    pragma import (c, from_surface_scaled, "AG_PixmapFromSurfaceScaled");

    function from_bitmap
      (parent : widget_access_t;
       path   : cs.chars_ptr) return pixmap_access_t;
    pragma import (c, from_bitmap, "AG_PixmapFromBMP");

--    function from_xcf
--      (parent : widget_access_t;
--       path   : cs.chars_ptr) return pixmap_access_t;
--    pragma import (c, from_xcf, "AG_PixmapFromXCF");
  end cbinds;

  function allocate
    (parent : widget_access_t;
     flags  : flags_t;
     width  : positive;
     height : positive) return pixmap_access_t is
  begin
    return cbinds.allocate
      (parent => parent,
       flags  => flags,
       width  => c.int (width),
       height => c.int (height));
  end allocate;

  function from_surface_scaled
    (parent : widget_access_t;
     flags  : flags_t;
     source : agar.gui.surface.surface_access_t;
     width  : positive;
     height : positive) return pixmap_access_t is
  begin
    return cbinds.from_surface_scaled
      (parent => parent,
       flags  => flags,
       source => source,
       width  => c.int (width),
       height => c.int (height));
  end from_surface_scaled;

  function from_bitmap
    (parent : widget_access_t;
     path   : string) return pixmap_access_t
  is
    c_path : aliased c.char_array := c.to_c (path);
  begin
    return cbinds.from_bitmap
      (parent => parent,
       path   => cs.to_chars_ptr (c_path'unchecked_access));
  end from_bitmap;

--  function from_xcf
--    (parent : widget_access_t;
--     path   : string) return pixmap_access_t
--  is
--    c_path : aliased c.char_array := c.to_c (path);
--  begin
--    return cbinds.from_xcf
--      (parent => parent,
--       path   => cs.to_chars_ptr (c_path'unchecked_access));
--  end from_xcf;

  function add_surface
    (pixmap  : pixmap_access_t;
     surface : agar.gui.surface.surface_access_t) return boolean is
  begin
    return cbinds.add_surface
      (pixmap  => pixmap,
       surface => surface) = 0;
  end add_surface;

  function add_surface_copy
    (pixmap  : pixmap_access_t;
     surface : agar.gui.surface.surface_access_t) return boolean is
  begin
    return cbinds.add_surface_copy
      (pixmap  => pixmap,
       surface => surface) = 0;
  end add_surface_copy;

  function add_surface_scaled
    (pixmap  : pixmap_access_t;
     surface : agar.gui.surface.surface_access_t;
     width   : positive;
     height  : positive) return boolean is
  begin
    return cbinds.add_surface_scaled
      (pixmap  => pixmap,
       surface => surface,
       width   => c.int (width),
       height  => c.int (height)) = 0;
  end add_surface_scaled;

  function widget (pixmap : pixmap_access_t) return widget_access_t is
  begin
    return pixmap.widget'access;
  end widget;

end agar.gui.widget.pixmap;
