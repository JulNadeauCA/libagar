package body agar.gui.widget is

  use type c.int;

  function size_allocate
    (widget : widget_access_t;
     alloc  : size_alloc_t) return boolean is
  begin
    return size_allocate (widget, alloc) = 0;
  end size_allocate;

  function enabled (widget : widget_access_t) return boolean is
  begin
    return enabled (widget) = 1;
  end enabled;

  function disabled (widget : widget_access_t) return boolean is
  begin
    return disabled (widget) = 1;
  end disabled;

  function focused (widget : widget_access_t) return boolean is
  begin
    return focused (widget) = 1;
  end focused;

  function area
    (widget : widget_access_t;
     x      : natural;
     y      : natural) return boolean is
  begin
    return area
      (widget => widget,
       x      => c.int (x),
       y      => c.int (y)) = 1;
  end area;

  function relative_area
    (widget : widget_access_t;
     x      : natural;
     y      : natural) return boolean is
  begin
    return relative_area
      (widget => widget,
       x      => c.int (x),
       y      => c.int (y)) = 1;
  end relative_area;

  procedure blit_from
    (dest_widget : widget_access_t;
     src_widget  : widget_access_t;
     surface_id  : surface_id_t;
     rect        : agar.gui.rect.rect_access_t;
     x           : integer;
     y           : integer) is
  begin
    blit_from
      (dest_widget => dest_widget,
       src_widget  => src_widget,
       surface_id  => surface_id,
       rect        => rect,
       x           => c.int (x),
       y           => c.int (y));
  end blit_from;

  procedure blit_surface
    (widget     : widget_access_t;
     surface_id : surface_id_t;
     x          : integer;
     y          : integer) is
  begin
    blit_surface
      (widget     => widget,
       surface_id => surface_id,
       x          => c.int (x),
       y          => c.int (y));
  end blit_surface;

  procedure put_pixel32
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     color  : agar.core.types.uint32_t) is
  begin
    put_pixel32
      (widget => widget,
       x      => c.int (x),
       y      => c.int (y),
       color  => color);
  end put_pixel32;

  procedure put_pixel32_or_clip
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     color  : agar.core.types.uint32_t) is
  begin
    put_pixel32_or_clip
      (widget => widget,
       x      => c.int (x),
       y      => c.int (y),
       color  => color);
  end put_pixel32_or_clip;

  procedure put_pixel_rgb
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     r      : agar.core.types.uint8_t;
     g      : agar.core.types.uint8_t;
     b      : agar.core.types.uint8_t) is
  begin
    put_pixel_rgb
      (widget => widget,
       x      => c.int (x),
       y      => c.int (y),
       r      => r, 
       g      => g,
       b      => b);
  end put_pixel_rgb;

  procedure put_pixel_rgb_or_clip
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     r      : agar.core.types.uint8_t;
     g      : agar.core.types.uint8_t;
     b      : agar.core.types.uint8_t) is
  begin
    put_pixel_rgb_or_clip
      (widget => widget,
       x      => c.int (x),
       y      => c.int (y),
       r      => r, 
       g      => g,
       b      => b);
  end put_pixel_rgb_or_clip;

  procedure blend_pixel_rgba
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     color  : color_t;
     func   : agar.gui.colors.blend_func_t) is
  begin
    blend_pixel_rgba
      (widget => widget,
       x      => c.int (x),
       y      => c.int (y),
       color  => color,
       func   => func);
  end blend_pixel_rgba;

  procedure blend_pixel_32
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     color  : agar.core.types.uint32_t;
     func   : agar.gui.colors.blend_func_t) is
  begin
    blend_pixel_32
      (widget => widget,
       x      => c.int (x),
       y      => c.int (y),
       color  => color,
       func   => func);
  end blend_pixel_32;

  function find_point
    (class_mask : string;
     x          : natural;
     y          : natural) return widget_access_t
  is
    ca_mask : aliased c.char_array := c.to_c (class_mask);
  begin
    return find_point
      (class_mask => cs.to_chars_ptr (ca_mask'unchecked_access),
       x          => c.int (x),
       y          => c.int (y));
  end find_point;

  function find_rect
    (class_mask : string;
     x          : natural;
     y          : natural;
     w          : positive;
     h          : positive) return widget_access_t
  is
    ca_mask : aliased c.char_array := c.to_c (class_mask);
  begin
    return find_rect
      (class_mask => cs.to_chars_ptr (ca_mask'unchecked_access),
       x          => c.int (x),
       y          => c.int (y),
       w          => c.int (w),
       h          => c.int (h));
  end find_rect;

end agar.gui.widget;
