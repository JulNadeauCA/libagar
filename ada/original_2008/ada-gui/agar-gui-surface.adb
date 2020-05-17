with interfaces.c.strings;

package body agar.gui.surface is
  package cs renames interfaces.c.strings;

  package cbinds is
    function allocate
      (width  : c.int;
       height : c.int;
       format : agar.gui.pixelformat.pixel_format_access_t;
       flags  : flags_t) return surface_access_t;
    pragma import (c, allocate, "AG_SurfaceNew");

    function std_rgb
      (width  : c.unsigned;
       height : c.unsigned) return surface_access_t;
    pragma import (c, std_rgb, "agar_surface_std_rgb");

    function std_rgba
      (width  : c.unsigned;
       height : c.unsigned) return surface_access_t;
    pragma import (c, std_rgba, "agar_surface_std_rgba");

    function indexed
      (width          : c.unsigned;
       height         : c.unsigned;
       bits_per_pixel : c.int;
       flags          : flags_t) return surface_access_t;
    pragma import (c, indexed, "AG_SurfaceIndexed");

    function rgb
      (width          : c.unsigned;
       height         : c.unsigned;
       bits_per_pixel : c.int;
       flags          : flags_t;
       rmask          : agar.core.types.uint32_t;
       gmask          : agar.core.types.uint32_t;
       bmask          : agar.core.types.uint32_t) return surface_access_t;
    pragma import (c, rgb, "AG_SurfaceRGB");

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

    function from_bmp (file : cs.chars_ptr) return surface_access_t;
    pragma import (c, from_bmp, "AG_SurfaceFromBMP");
  end cbinds;

  function allocate
    (width  : positive;
     height : positive;
     format : agar.gui.pixelformat.pixel_format_access_t;
     flags  : flags_t) return surface_access_t is
  begin
    return cbinds.allocate
      (width  => c.int (width),
       height => c.int (height),
       format => format,
       flags  => flags);
  end allocate;

  function std_rgb
    (width  : positive;
     height : positive) return surface_access_t is
  begin
    return cbinds.std_rgb
      (width  => c.unsigned (width),
       height => c.unsigned (height));
  end std_rgb;

  function std_rgba
    (width  : positive;
     height : positive) return surface_access_t is
  begin
    return cbinds.std_rgba
      (width  => c.unsigned (width),
       height => c.unsigned (height));
  end std_rgba;

  function indexed
    (width          : positive;
     height         : positive;
     bits_per_pixel : positive;
     flags          : flags_t) return surface_access_t is
  begin
    return cbinds.indexed
      (width          => c.unsigned (width),
       height         => c.unsigned (height),
       bits_per_pixel => c.int (bits_per_pixel),
       flags          => flags);
  end indexed;

  function rgb
    (width          : positive;
     height         : positive;
     bits_per_pixel : positive;
     flags          : flags_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t) return surface_access_t is
  begin
    return cbinds.rgb
      (width          => c.unsigned (width),
       height         => c.unsigned (height),
       bits_per_pixel => c.int (bits_per_pixel),
       flags          => flags,
       rmask          => rmask,
       gmask          => gmask,
       bmask          => bmask);
  end rgb;

  function rgba
    (width          : positive;
     height         : positive;
     bits_per_pixel : positive;
     flags          : flags_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t;
     amask          : agar.core.types.uint32_t) return surface_access_t is
  begin
    return cbinds.rgba
      (width          => c.unsigned (width),
       height         => c.unsigned (height),
       bits_per_pixel => c.int (bits_per_pixel),
       flags          => flags,
       rmask          => rmask,
       gmask          => gmask,
       bmask          => bmask,
       amask          => amask);
  end rgba;

  function from_pixels_rgb
    (pixels         : agar.core.types.void_ptr_t;
     width          : positive;
     height         : positive;
     bits_per_pixel : positive;
     flags          : flags_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t) return surface_access_t is
  begin
    return cbinds.from_pixels_rgb
      (pixels         => pixels,
       width          => c.unsigned (width),
       height         => c.unsigned (height),
       bits_per_pixel => c.int (bits_per_pixel),
       flags          => flags,
       rmask          => rmask,
       gmask          => gmask,
       bmask          => bmask);
  end from_pixels_rgb;

  function from_pixels_rgba
    (pixels         : agar.core.types.void_ptr_t;
     width          : positive;
     height         : positive;
     bits_per_pixel : positive;
     flags          : flags_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t;
     amask          : agar.core.types.uint32_t) return surface_access_t is
  begin
    return cbinds.from_pixels_rgba
      (pixels         => pixels,
       width          => c.unsigned (width),
       height         => c.unsigned (height),
       bits_per_pixel => c.int (bits_per_pixel),
       flags          => flags,
       rmask          => rmask,
       gmask          => gmask,
       bmask          => bmask,
       amask          => amask);
  end from_pixels_rgba;

  function from_bmp (file : string) return surface_access_t is
    ca_file : aliased c.char_array := c.to_c (file);
  begin
    return cbinds.from_bmp (cs.to_chars_ptr (ca_file'unchecked_access));
  end from_bmp;

end agar.gui.surface;
