package body agar.gui.widget is

  use type c.int;

  function size_allocate
    (widget : widget_access_t;
     alloc  : size_alloc_t) return c.int;
  pragma import (c, size_allocate, "AG_WidgetSizeAlloc");

  function size_allocate
    (widget : widget_access_t;
     alloc  : size_alloc_t) return boolean is
  begin
    return size_allocate (widget, alloc) = 0;
  end size_allocate;

  function enabled (widget : widget_access_t) return c.int;
  pragma import (c, enabled, "agar_widget_enabled");

  function enabled (widget : widget_access_t) return boolean is
  begin
    return enabled (widget) = 1;
  end enabled;

  function disabled (widget : widget_access_t) return c.int;
  pragma import (c, disabled, "agar_widget_disabled");

  function disabled (widget : widget_access_t) return boolean is
  begin
    return disabled (widget) = 1;
  end disabled;

  function focused (widget : widget_access_t) return c.int;
  pragma import (c, focused, "agar_widget_focused");

  function focused (widget : widget_access_t) return boolean is
  begin
    return focused (widget) = 1;
  end focused;

  procedure blit
    (widget  : widget_access_t;
     surface : agar.gui.surface.surface_access_t;
     x       : c.int;
     y       : c.int);
  pragma import (c, blit, "AG_WidgetBlit");

  procedure blit
    (widget  : widget_access_t;
     surface : agar.gui.surface.surface_access_t;
     x       : natural;
     y       : natural) is
  begin
    blit
      (widget  => widget,
       surface => surface,
       x       => c.int (x),
       y       => c.int (y));
  end blit;
 
  procedure blit_from
    (dest_widget : widget_access_t;
     src_widget  : widget_access_t;
     surface_id  : surface_id_t;
     rect        : agar.gui.rect.rect_access_t;
     x           : c.int;
     y           : c.int);
  pragma import (c, blit_from, "AG_WidgetBlitFrom");

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
     x          : c.int;
     y          : c.int);
  pragma import (c, blit_surface, "agar_widget_blit_surface");

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
  
  procedure put_pixel
    (widget : widget_access_t;
     x      : c.int;
     y      : c.int;
     color  : agar.core.types.color_t);
  pragma import (c, put_pixel, "agar_widget_put_pixel");

  procedure put_pixel
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     color  : agar.core.types.color_t) is
  begin
    put_pixel
      (widget => widget,
       x      => c.int (x),
       y      => c.int (y),
       color  => color);
  end put_pixel;

  procedure put_pixel32
    (widget : widget_access_t;
     x      : c.int;
     y      : c.int;
     color  : agar.core.types.uint32_t);
  pragma import (c, put_pixel32, "agar_widget_put_pixel32");

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

  procedure put_pixel_rgb
    (widget : widget_access_t;
     x      : c.int;
     y      : c.int;
     r      : agar.core.types.uint8_t;
     g      : agar.core.types.uint8_t;
     b      : agar.core.types.uint8_t); 
  pragma import (c, put_pixel_rgb, "agar_widget_put_pixel_rgb");

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
  
  procedure blend_pixel
    (widget : widget_access_t;
     x      : c.int;
     y      : c.int;
     color  : color_t;
     func   : agar.gui.colors.blend_func_t);
  pragma import (c, blend_pixel, "AG_BlendPixel");

  procedure blend_pixel
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     color  : color_t;
     func   : agar.gui.colors.blend_func_t) is
  begin
    blend_pixel
      (widget => widget,
       x      => c.int (x),
       y      => c.int (y),
       color  => color,
       func   => func);
  end blend_pixel

  procedure blend_pixel_32
    (widget : widget_access_t;
     x      : c.int;
     y      : c.int;
     color  : agar.core.types.uint32_t;
     func   : agar.gui.colors.blend_func_t);
  pragma import (c, blend_pixel_32, "agar_widget_blend_pixel_32");

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
    (class_mask : cs.chars_ptr;
     x          : c.int;
     y          : c.int) return widget_access_t;
  pragma import (c, find_point, "AG_WidgetFindPoint");

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
    (class_mask : cs.chars_ptr;
     x          : c.int;
     y          : c.int;
     w          : c.int;
     h          : c.int) return widget_access_t;
  pragma import (c, find_rect, "AG_WidgetFindRect");

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

  function absolute_coords_inside
    (widget : widget_access_t;
     x      : c.int;
     y      : c.int) return c.int;
  pragma import (c, absolute_coords_inside, "agar_widget_area");

  function absolute_coords_inside
    (widget : widget_access_t;
     x      : natural;
     y      : natural) return boolean is
  begin
    return absolute_coords_inside
      (widget => widget,
       x      => c.int (x),
       y      => c.int (y)) = 1;
  end absolute_coords_inside;

  function relative_coords_inside
    (widget : widget_access_t;
     x      : c.int;
     y      : c.int) return c.int;
  pragma import (c, relative_coords_inside, "agar_widget_relative_area");

  function relative_coords_inside
    (widget : widget_access_t;
     x      : natural;
     y      : natural) return boolean is
  begin
    return relative_coords_inside
      (widget => widget,
       x      => c.int (x),
       y      => c.int (y)) = 1;
  end relative_coords_inside;

  --

  procedure set_position
    (widget : widget_access_t;
     x      : natural;
     y      : natural) is
  begin
    widget.x := c.int (x);
    widget.y := c.int (y);
  end set_position;

  procedure modify_position
    (widget : widget_access_t;
     x      : integer := 0;
     y      : integer := 0) is
  begin
    widget.x := widget.x + c.int (x);
    widget.y := widget.y + c.int (y);
  end modify_position;

  procedure set_size
    (widget : widget_access_t;
     width  : positive;
     height : positive) is
  begin
    widget.w := c.int (width);
    widget.h := c.int (height);
  end set_size;

  procedure modify_size
    (widget : widget_access_t;
     width  : integer := 0;
     height : integer := 0) is
  begin
    widget.w := widget.w + c.int (width);
    widget.h := widget.h + c.int (height);
  end modify_size;

  function object (widget : widget_access_t) return agar.core.object.object_access_t is
  begin
    return widget.object'access;
  end object;

  -- binding procedures
  package body bindings is
    package cbinds is
      procedure bind_pointer
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.void_ptr_t);
      pragma import (c, bind_pointer, "agar_gui_widget_bind_pointer");

      procedure bind_property
        (widget  : widget_access_t;
         binding : cs.chars_ptr;
         object  : agar.core.object.object_access_t;
         name    : cs.chars_ptr);
      pragma import (c, bind_property, "agar_gui_widget_bind_property");

      procedure bind_boolean
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.boolean_access_t);
      pragma import (c, bind_boolean, "agar_gui_widget_bind_boolean");

      procedure bind_integer
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.integer_access_t);
      pragma import (c, bind_integer, "agar_gui_widget_bind_integer");

      procedure bind_unsigned
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.unsigned_access_t);
      pragma import (c, bind_unsigned, "agar_gui_widget_bind_unsigned");

      procedure bind_float
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.float_access_t);
      pragma import (c, bind_float, "agar_gui_widget_bind_float");

      procedure bind_double
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.double_access_t);
      pragma import (c, bind_double, "agar_gui_widget_bind_double");

      procedure bind_uint8
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.uint8_ptr_t);
      pragma import (c, bind_uint8, "agar_gui_widget_bind_uint8");

      procedure bind_int8
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.int8_ptr_t);
      pragma import (c, bind_int8, "agar_gui_widget_bind_int8");

      procedure bind_flag8
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.uint8_ptr_t;
         mask     : agar.core.types.uint8_t);
      pragma import (c, bind_flag8, "agar_gui_widget_bind_flag8");

      procedure bind_uint16
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.uint16_ptr_t);
      pragma import (c, bind_uint16, "agar_gui_widget_bind_uint16");

      procedure bind_int16
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.int16_ptr_t);
      pragma import (c, bind_int16, "agar_gui_widget_bind_int16");

      procedure bind_flag16
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.uint16_ptr_t;
         mask     : agar.core.types.uint16_t);
      pragma import (c, bind_flag16, "agar_gui_widget_bind_flag16");

      procedure bind_uint32
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.uint32_ptr_t);
      pragma import (c, bind_uint32, "agar_gui_widget_bind_uint32");

      procedure bind_int32
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.int32_ptr_t);
      pragma import (c, bind_int32, "agar_gui_widget_bind_int32");

      procedure bind_flag32
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.uint32_ptr_t;
         mask     : agar.core.types.uint32_t);
      pragma import (c, bind_flag32, "agar_gui_widget_bind_flag32");

      -- get

      function get_pointer
        (widget   : widget_access_t;
         binding  : cs.chars_ptr) return agar.core.types.void_ptr_t;
      pragma import (c, get_pointer, "agar_gui_widget_get_pointer");

      function get_boolean
        (widget   : widget_access_t;
         binding  : cs.chars_ptr) return agar.core.types.boolean_t;
      pragma import (c, get_boolean, "agar_gui_widget_get_boolean");

      function get_integer
        (widget   : widget_access_t;
         binding  : cs.chars_ptr) return agar.core.types.integer_t;
      pragma import (c, get_integer, "agar_gui_widget_get_integer");

      function get_unsigned
        (widget   : widget_access_t;
         binding  : cs.chars_ptr) return agar.core.types.unsigned_t;
      pragma import (c, get_unsigned, "agar_gui_widget_get_unsigned");

      function get_float
        (widget   : widget_access_t;
         binding  : cs.chars_ptr) return agar.core.types.float_t;
      pragma import (c, get_float, "agar_gui_widget_get_float");

      function get_double
        (widget   : widget_access_t;
         binding  : cs.chars_ptr) return agar.core.types.double_t;
      pragma import (c, get_double, "agar_gui_widget_get_double");

      function get_uint8
        (widget   : widget_access_t;
         binding  : cs.chars_ptr) return agar.core.types.uint8_t;
      pragma import (c, get_uint8, "agar_gui_widget_get_uint8");

      function get_int8
        (widget   : widget_access_t;
         binding  : cs.chars_ptr) return agar.core.types.int8_t;
      pragma import (c, get_int8, "agar_gui_widget_get_int8");

      function get_uint16
        (widget   : widget_access_t;
         binding  : cs.chars_ptr) return agar.core.types.uint16_t;
      pragma import (c, get_uint16, "agar_gui_widget_get_uint16");

      function get_int16
        (widget   : widget_access_t;
         binding  : cs.chars_ptr) return agar.core.types.int16_t;
      pragma import (c, get_int16, "agar_gui_widget_get_int16");

      function get_uint32
        (widget   : widget_access_t;
         binding  : cs.chars_ptr) return agar.core.types.uint32_t;
      pragma import (c, get_uint32, "agar_gui_widget_get_uint32");

      function get_int32
        (widget   : widget_access_t;
         binding  : cs.chars_ptr) return agar.core.types.int32_t;
      pragma import (c, get_int32, "agar_gui_widget_get_int32");

      -- set

      procedure set_pointer
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.void_ptr_t);
      pragma import (c, set_pointer, "agar_gui_widget_set_pointer");


      procedure set_boolean
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.boolean_t);
      pragma import (c, set_boolean, "agar_gui_widget_set_boolean");


      procedure set_integer
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.integer_t);
      pragma import (c, set_integer, "agar_gui_widget_set_integer");


      procedure set_unsigned
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.unsigned_t);
      pragma import (c, set_unsigned, "agar_gui_widget_set_unsigned");


      procedure set_float
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.float_t);
      pragma import (c, set_float, "agar_gui_widget_set_float");


      procedure set_double
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.double_t);
      pragma import (c, set_double, "agar_gui_widget_set_double");

      procedure set_uint8
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.uint8_t);
      pragma import (c, set_uint8, "agar_gui_widget_set_uint8");

      procedure set_int8
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.int8_t);
      pragma import (c, set_int8, "agar_gui_widget_set_int8");

      procedure set_uint16
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.uint16_t);
      pragma import (c, set_uint16, "agar_gui_widget_set_uint16");

      procedure set_int16
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.int16_t);
      pragma import (c, set_int16, "agar_gui_widget_set_int16");

      procedure set_uint32
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.uint32_t);
      pragma import (c, set_uint32, "agar_gui_widget_set_uint32");

      procedure set_int32
        (widget   : widget_access_t;
         binding  : cs.chars_ptr;
         variable : agar.core.types.int32_t);
      pragma import (c, set_int32, "agar_gui_widget_set_int32");

    end cbinds;

    procedure bind_pointer
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.void_ptr_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_pointer
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end bind_pointer;

    procedure bind_property
      (widget  : widget_access_t;
       binding : string;
       object  : agar.core.object.object_access_t;
       name    : string)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
      c_name    : aliased c.char_array := c.to_c (name);
    begin
      cbinds.bind_property
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access),
         object  => object,
         name    => cs.to_chars_ptr (c_name'unchecked_access));
    end bind_property;

    procedure bind_boolean
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.boolean_access_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_boolean
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end bind_boolean;

    procedure bind_integer
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.integer_access_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_integer
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end bind_integer;

    procedure bind_unsigned
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.unsigned_access_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_unsigned
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end bind_unsigned;

    procedure bind_float
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.float_access_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_float
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end bind_float;

    procedure bind_double
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.double_access_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_double
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end bind_double;

    procedure bind_uint8
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint8_ptr_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_uint8
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end bind_uint8;

    procedure bind_int8
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.int8_ptr_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_int8
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end bind_int8;

    procedure bind_flag8
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint8_ptr_t;
       mask     : agar.core.types.uint8_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_flag8
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable,
         mask     => mask);
    end bind_flag8;

    procedure bind_uint16
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint16_ptr_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_uint16
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end bind_uint16;

    procedure bind_int16
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.int16_ptr_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_int16
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end bind_int16;

    procedure bind_flag16
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint16_ptr_t;
       mask     : agar.core.types.uint16_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_flag16
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable,
         mask     => mask);
    end bind_flag16;

    procedure bind_uint32
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint32_ptr_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_uint32
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end bind_uint32;

    procedure bind_int32
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.int32_ptr_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_int32
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end bind_int32;

    procedure bind_flag32
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint32_ptr_t;
       mask     : agar.core.types.uint32_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.bind_flag32
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable,
         mask     => mask);
    end bind_flag32;

    -- get

    function get_pointer
      (widget  : widget_access_t;
       binding : string) return agar.core.types.void_ptr_t
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      return cbinds.get_pointer
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access));
    end get_pointer;

    function get_boolean
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.boolean_t
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      return cbinds.get_boolean
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access));
    end get_boolean;

    function get_integer
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.integer_t
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      return cbinds.get_integer
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access));
    end get_integer;

    function get_unsigned
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.unsigned_t
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      return cbinds.get_unsigned
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access));
    end get_unsigned;

    function get_float
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.float_t
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      return cbinds.get_float
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access));
    end get_float;

    function get_double
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.double_t
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      return cbinds.get_double
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access));
    end get_double;

    function get_uint8
      (widget  : widget_access_t;
       binding : string) return agar.core.types.uint8_t
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      return cbinds.get_uint8
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access));
    end get_uint8;

    function get_int8
      (widget  : widget_access_t;
       binding : string) return agar.core.types.int8_t
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      return cbinds.get_int8
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access));
    end get_int8;

    function get_uint16
      (widget  : widget_access_t;
       binding : string) return agar.core.types.uint16_t
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      return cbinds.get_uint16
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access));
    end get_uint16;

    function get_int16
      (widget  : widget_access_t;
       binding : string) return agar.core.types.int16_t
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      return cbinds.get_int16
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access));
    end get_int16;

    function get_uint32
      (widget  : widget_access_t;
       binding : string) return agar.core.types.uint32_t
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      return cbinds.get_uint32
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access));
    end get_uint32;

    function get_int32
      (widget  : widget_access_t;
       binding : string) return agar.core.types.int32_t
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      return cbinds.get_int32
        (widget  => widget,
         binding => cs.to_chars_ptr (c_binding'unchecked_access));
    end get_int32;

    -- set

    procedure set_pointer
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.void_ptr_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.set_pointer
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end set_pointer;

    procedure set_boolean
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.boolean_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.set_boolean
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end set_boolean;

    procedure set_integer
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.integer_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.set_integer
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end set_integer;

    procedure set_unsigned
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.unsigned_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.set_unsigned
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end set_unsigned;

    procedure set_float
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.float_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.set_float
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end set_float;

    procedure set_double
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.double_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.set_double
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end set_double;

    procedure set_uint8
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint8_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.set_uint8
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end set_uint8;

    procedure set_int8
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.int8_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.set_int8
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end set_int8;

    procedure set_uint16
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint16_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.set_uint16
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end set_uint16;

    procedure set_int16
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.int16_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.set_int16
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end set_int16;

    procedure set_uint32
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint32_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.set_uint32
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end set_uint32;

    procedure set_int32
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.int32_t)
    is
      c_binding : aliased c.char_array := c.to_c (binding);
    begin
      cbinds.set_int32
        (widget   => widget,
         binding  => cs.to_chars_ptr (c_binding'unchecked_access),
         variable => variable);
    end set_int32;

  end bindings;

end agar.gui.widget;
