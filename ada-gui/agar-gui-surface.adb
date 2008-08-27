package body agar.gui.surface is

  function allocate
    (width  : positive;
     height : positive;
     format : agar.gui.pixelformat.pixel_format_access_t;
     flags  : flags_t) return surface_access_t is
  begin
    return allocate
      (width  => c.int (width),
       height => c.int (height),
       format => format,
       flags  => flags);
  end allocate;

  function std_rgb
    (width  : positive;
     height : positive) return surface_access_t is
  begin
    return std_rgb
      (width  => c.unsigned (width),
       height => c.unsigned (height));
  end std_rgb;

  function std_rgba
    (width  : positive;
     height : positive) return surface_access_t is
  begin
    return std_rgba
      (width  => c.unsigned (width),
       height => c.unsigned (height));
  end std_rgba;

  function indexed
    (width          : positive;
     height         : positive;
     bits_per_pixel : positive;
     flags          : flags_t) return surface_access_t is
  begin
    return indexed
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
    return rgb
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
    return rgba
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
    return from_pixels_rgb
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
    return from_pixels_rgba
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
    return from_bmp (cs.to_chars_ptr (ca_file'unchecked_access));
  end from_bmp;

end agar.gui.surface;
