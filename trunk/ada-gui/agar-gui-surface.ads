with SDL.video;
with SDL;
with agar.core.types;
with agar.core;
with agar.gui.pixelformat;
with interfaces.c.strings;

package agar.gui.surface is
  package cs renames interfaces.c.strings;

  -- types
  type surface_t is new SDL.video.surface_t;
  type surface_access_t is access all surface_t;
  pragma convention (c, surface_t);

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
    (width  : c.int;
     height : c.int;
     format : agar.gui.pixelformat.pixel_format_access_t;
     flags  : flags_t) return surface_access_t;
  pragma import (c, allocate, "AG_SurfaceNew");

  function allocate
    (width  : positive;
     height : positive;
     format : agar.gui.pixelformat.pixel_format_access_t;
     flags  : flags_t) return surface_access_t;
  pragma inline (allocate);

  function empty return surface_access_t;
  pragma import (c, empty, "AG_SurfaceEmpty");

  function std_rgb
    (width  : c.unsigned;
     height : c.unsigned) return surface_access_t;
  pragma import (c, std_rgb, "agar_surface_std_rgb");

  function std_rgb
    (width  : positive;
     height : positive) return surface_access_t;
  pragma inline (std_rgb);

  function std_rgba
    (width  : c.unsigned;
     height : c.unsigned) return surface_access_t;
  pragma import (c, std_rgba, "agar_surface_std_rgba");

  function std_rgba
    (width  : positive;
     height : positive) return surface_access_t;
  pragma inline (std_rgba);

  function indexed
    (width          : c.unsigned;
     height         : c.unsigned;
     bits_per_pixel : c.int;
     flags          : flags_t) return surface_access_t;
  pragma import (c, indexed, "AG_SurfaceIndexed");

  function indexed
    (width          : positive;
     height         : positive;
     bits_per_pixel : positive;
     flags          : flags_t) return surface_access_t;
  pragma inline (indexed);

  function rgb
    (width          : c.unsigned;
     height         : c.unsigned;
     bits_per_pixel : c.int;
     flags          : flags_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t) return surface_access_t;
  pragma import (c, rgb, "AG_SurfaceRGB");

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
    (width          : c.unsigned;
     height         : c.unsigned;
     bits_per_pixel : c.int;
     flags          : flags_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t;
     amask          : agar.core.types.uint32_t) return surface_access_t;
  pragma import (c, rgba, "AG_SurfaceRGBA");
 
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
     width          : c.unsigned;
     height         : c.unsigned;
     bits_per_pixel : c.int;
     flags          : flags_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t) return surface_access_t;
  pragma import (c, from_pixels_rgb, "AG_SurfaceFromPixelsRGB");
 
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
     width          : c.unsigned;
     height         : c.unsigned;
     bits_per_pixel : c.int;
     flags          : flags_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t;
     amask          : agar.core.types.uint32_t) return surface_access_t;
  pragma import (c, from_pixels_rgba, "AG_SurfaceFromPixelsRGBA");
 
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
 
  function from_bmp (file : cs.chars_ptr) return surface_access_t;
  pragma import (c, from_bmp, "AG_SurfaceFromBMP");

  function from_bmp (file : string) return surface_access_t;
  pragma inline (from_bmp);

  function from_sdl (surface : sdl.video.surface_ptr_t) return surface_access_t;
  pragma import (c, from_sdl, "AG_SurfaceFromSDL");

  procedure free (surface : surface_access_t);
  pragma import (c, free, "AG_SurfaceFree");

  procedure copy
    (dest : surface_access_t;
     src  : surface_access_t);
  pragma import (c, copy, "AG_SurfaceCopy");

end agar.gui.surface;
