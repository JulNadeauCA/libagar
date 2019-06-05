with SDL.video;
with SDL;
with agar.core.types;
with agar.core;

package agar.gui.pixelformat is

  type pixel_format_t is new SDL.video.pixel_format_t;
  type pixel_format_access_t is access all pixel_format_t;

  function rgb
    (bits_per_pixel : agar.core.types.uint8_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t) return pixel_format_access_t;
  pragma import (c, rgb, "AG_PixelFormatRGB");

  function rgba
    (bits_per_pixel : agar.core.types.uint8_t;
     rmask          : agar.core.types.uint32_t;
     gmask          : agar.core.types.uint32_t;
     bmask          : agar.core.types.uint32_t;
     amask          : agar.core.types.uint32_t) return pixel_format_access_t;
  pragma import (c, rgba, "AG_PixelFormatRGBA");

  function indexed
    (bits_per_pixel : agar.core.types.uint8_t) return pixel_format_access_t;
  pragma import (c, indexed, "AG_PixelFormatIndexed");

  procedure free (format : pixel_format_access_t);
  pragma import (c, free, "AG_PixelFormatFree");

end agar.gui.pixelformat;
