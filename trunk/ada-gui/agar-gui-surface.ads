with SDL.video;
with SDL;
with agar.core.types;
with agar.core;
with agar.gui.pixelformat;

package agar.gui.surface is

  -- types
  type surface_t is new SDL.video.surface_t;
  type surface_access_t is access all surface_t;
  pragma convention (c, surface_t);

  type index_t is new c.int;
  type index_access_t is access all index_t;
  pragma convention (c, index_t);
  pragma convention (c, index_access_t);

  -- constants
  type flags_t is new c.unsigned;
  pragma convention (c, flags_t);

  HWSURFACE   : constant flags_t := flags_t (SDL.video.HWSURFACE);
  SRCCOLORKEY : constant flags_t := flags_t (SDL.video.SRCCOLORKEY);
  SRCALPHA    : constant flags_t := flags_t (SDL.video.SRCALPHA);

  --
  -- API
  --

  function allocate
    (width  : positive;
     height : positive;
     format : agar.gui.pixelformat.pixel_format_access_t;
     flags  : flags_t) return surface_access_t;
  pragma inline (allocate);

  function empty return surface_access_t;
  pragma import (c, empty, "AG_SurfaceEmpty");

  function std_rgb
    (width  : positive;
     height : positive) return surface_access_t;
  pragma inline (std_rgb);

  function std_rgba
    (width  : positive;
     height : positive) return surface_access_t;
  pragma inline (std_rgba);

  function indexed
    (width          : positive;
     height         : positive;
     bits_per_pixel : positive;
     flags          : flags_t) return surface_access_t;
  pragma inline (indexed);

  function rgb
    (width          : positive;
     height         : positive;
     bits_per_pixel : positive;
     flags          : flags_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t) return surface_access_t;
  pragma inline (rgb);

  function rgba
    (width          : positive;
     height         : positive;
     bits_per_pixel : positive;
     flags          : flags_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t;
     amask          : agar.core.types.uint32_t) return surface_access_t;
  pragma inline (rgba);

  function from_pixels_rgb
    (pixels         : agar.core.types.void_ptr_t;
     width          : positive;
     height         : positive;
     bits_per_pixel : positive;
     flags          : flags_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t) return surface_access_t;
  pragma inline (from_pixels_rgb);
 
  function from_pixels_rgba
    (pixels         : agar.core.types.void_ptr_t;
     width          : positive;
     height         : positive;
     bits_per_pixel : positive;
     flags          : flags_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t;
     amask          : agar.core.types.uint32_t) return surface_access_t;
  pragma inline (from_pixels_rgba);
 
  function from_bmp (file : string) return surface_access_t;
  pragma inline (from_bmp);

  function from_sdl (surface : sdl.video.surface_access_t) return surface_access_t;
  pragma import (c, from_sdl, "AG_SurfaceFromSDL");

  procedure free (surface : surface_access_t);
  pragma import (c, free, "AG_SurfaceFree");

  procedure copy
    (dest : surface_access_t;
     src  : surface_access_t);
  pragma import (c, copy, "AG_SurfaceCopy");

end agar.gui.surface;
