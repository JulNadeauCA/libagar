with agar.core.object;
with agar.core.slist;
with agar.core.threads;
with agar.core.types;
with agar.gui.colors;
with agar.gui.rect;
with agar.gui.surface;
with interfaces.c.strings;

package agar.gui.widget is
  package cs renames interfaces.c.strings;

  --
  -- forward declarations
  --

  type widget_t;
  type widget_access_t is access all widget_t;
  pragma convention (c, widget_access_t);

  type binding_t;
  type binding_access_t is access all binding_t;
  pragma convention (c, binding_access_t);

  -- this is necessary because of a circular dependency issue
  -- (widget -> menu -> widget)
  type fake_popup_menu_access_t is access all agar.core.types.void_ptr_t;
  pragma convention (c, fake_popup_menu_access_t);

  package binding_slist is new agar.core.slist
    (entry_type => binding_access_t);
  package menu_slist is new agar.core.slist
    (entry_type => fake_popup_menu_access_t);

  --
  -- constants
  --

  binding_name_max : constant c.unsigned := 16;

  --
  -- types
  --

  type size_req_t is record
    w : c.int;
    h : c.int;
  end record;
  type size_req_access_t is access all size_req_t;
  pragma convention (c, size_req_t);
  pragma convention (c, size_req_access_t);

  type size_alloc_t is record
    w : c.int;
    h : c.int;
    x : c.int;
    y : c.int;
  end record;
  type size_alloc_access_t is access all size_alloc_t;
  pragma convention (c, size_alloc_t);
  pragma convention (c, size_alloc_access_t);

  type class_t is record
    inherit       : agar.core.object.class_t;
    draw          : access procedure (vp : agar.core.types.void_ptr_t);
    size_request  : access procedure
      (vp  : agar.core.types.void_ptr_t;
       req : size_req_access_t);
    size_allocate : access function
      (vp    : agar.core.types.void_ptr_t;
       alloc : size_alloc_access_t) return c.int;
  end record;
  type class_access_t is access all class_t;
  pragma convention (c, class_t);
  pragma convention (c, class_access_t);

  type binding_type_t is (
    WIDGET_NONE,
    WIDGET_BOOL,
    WIDGET_UINT,
    WIDGET_INT,
    WIDGET_UINT8,
    WIDGET_SINT8,
    WIDGET_UINT16,
    WIDGET_SINT16,
    WIDGET_UINT32,
    WIDGET_SINT32,
    WIDGET_UINT64,
    WIDGET_SINT64,
    WIDGET_FLOAT,
    WIDGET_DOUBLE,
    WIDGET_LONG_DOUBLE,
    WIDGET_STRING,
    WIDGET_POINTER,
    WIDGET_PROP,
    WIDGET_FLAG,
    WIDGET_FLAG8,
    WIDGET_FLAG16,
    WIDGET_FLAG32
  );
  for binding_type_t use (
    WIDGET_NONE        => 0,
    WIDGET_BOOL        => 1,
    WIDGET_UINT        => 2,
    WIDGET_INT         => 3,
    WIDGET_UINT8       => 4,
    WIDGET_SINT8       => 5,
    WIDGET_UINT16      => 6,
    WIDGET_SINT16      => 7,
    WIDGET_UINT32      => 8,
    WIDGET_SINT32      => 9,
    WIDGET_UINT64      => 10,
    WIDGET_SINT64      => 11,
    WIDGET_FLOAT       => 12,
    WIDGET_DOUBLE      => 13,
    WIDGET_LONG_DOUBLE => 14,
    WIDGET_STRING      => 15,
    WIDGET_POINTER     => 16,
    WIDGET_PROP        => 17,
    WIDGET_FLAG        => 18,
    WIDGET_FLAG8       => 19,
    WIDGET_FLAG16      => 20,
    WIDGET_FLAG32      => 21
  );
  for binding_type_t'size use c.unsigned'size;
  pragma convention (c, binding_type_t);

  type size_spec_t is (
    WIDGET_BAD_SPEC,
    WIDGET_PIXELS,
    WIDGET_PERCENT,
    WIDGET_STRINGLEN,
    WIDGET_FILL
  );
  for size_spec_t use (
    WIDGET_BAD_SPEC  => 0,
    WIDGET_PIXELS    => 1,
    WIDGET_PERCENT   => 2,
    WIDGET_STRINGLEN => 3,
    WIDGET_FILL      => 4
  );
  for size_spec_t'size use c.unsigned'size;
  pragma convention (c, size_spec_t);

  type flag_descr_t is record
    bitmask   : c.unsigned;
    descr     : cs.chars_ptr;
    writeable : c.int;
  end record;
  type flag_descr_access_t is access all flag_descr_t;
  pragma convention (c, flag_descr_t);
  pragma convention (c, flag_descr_access_t);

  type binding_name_t is array (1 .. binding_name_max) of aliased c.char;
  pragma convention (c, binding_name_t);
  type binding_data_prop_t is array (1 .. agar.core.object.prop_key_max) of aliased c.char;
  pragma convention (c, binding_data_prop_t);
  type binding_data_union_selector_t is (prop, size, mask);
  type binding_data_union_t (selector : binding_data_union_selector_t := prop) is record
    case selector is
      when prop => prop : binding_data_prop_t;
      when size => size : c.size_t;
      when mask => mask : agar.core.types.uint32_t;
    end case;
  end record;
  pragma convention (c, binding_data_union_t);
  pragma unchecked_union (binding_data_union_t);

  type binding_t is record
    name         : binding_name_t;
    binding_type : c.int;
    mutex        : agar.core.threads.mutex_t;
    p1           : agar.core.types.void_ptr_t;
    data         : binding_data_union_t;
    bindings     : binding_slist.entry_t;
  end record;
  pragma convention (c, binding_t);

  type color_t is array (1 .. 4) of aliased agar.core.types.uint8_t;
  pragma convention (c, color_t);

  type surface_id_t is new c.int;
  pragma convention (c, surface_id_t);

  subtype flags_t is c.unsigned;

  -- widget type
  type widget_private_t is limited private;
  type widget_t is record
    object           : aliased agar.core.object.object_t;
    flags            : c.unsigned;

    x                : c.int;
    y                : c.int;
    w                : c.int;
    h                : c.int;

    privdata         : widget_private_t;
  end record;
  pragma convention (c, widget_t);

  --
  -- API
  --

  procedure expand (widget : widget_access_t);
  pragma import (c, expand, "agar_widget_expand");

  procedure expand_horizontal (widget : widget_access_t);
  pragma import (c, expand_horizontal, "agar_widget_expand_horizontal");

  procedure expand_vertical (widget : widget_access_t);
  pragma import (c, expand_vertical, "agar_widget_expand_vertical");

  procedure size_request
    (widget  : widget_access_t;
     request : size_req_t);
  pragma import (c, size_request, "AG_WidgetSizeReq");

  function size_allocate
    (widget : widget_access_t;
     alloc  : size_alloc_t) return boolean;
  pragma inline (size_allocate);

  -- input state

  procedure enable (widget : widget_access_t);
  pragma import (c, enable, "agar_widget_enable");

  procedure disable (widget : widget_access_t);
  pragma import (c, disable, "agar_widget_disable");

  function enabled (widget : widget_access_t) return boolean;
  pragma inline (enabled);

  function disabled (widget : widget_access_t) return boolean;
  pragma inline (disabled);

  -- focus

  function focused (widget : widget_access_t) return boolean;
  pragma inline (focused);

  procedure focus (widget : widget_access_t);
  pragma import (c, focus, "AG_WidgetFocus");

  procedure unfocus (widget : widget_access_t);
  pragma import (c, unfocus, "AG_WidgetUnfocus");

  -- missing: find_focused moved to agar.gui.window

  -- blitting

  function map_surface
    (widget  : widget_access_t;
     surface : agar.gui.surface.surface_access_t) return surface_id_t;
  pragma import (c, map_surface, "AG_WidgetMapSurface");

  function map_surface_no_copy
    (widget  : widget_access_t;
     surface : agar.gui.surface.surface_access_t) return surface_id_t;
  pragma import (c, map_surface_no_copy, "AG_WidgetMapSurfaceNODUP");

  procedure replace_surface
    (widget     : widget_access_t;
     surface_id : surface_id_t;
     surface    : agar.gui.surface.surface_access_t);
  pragma import (c, replace_surface, "AG_WidgetReplaceSurface");

  procedure replace_surface_no_copy
    (widget     : widget_access_t;
     surface_id : surface_id_t;
     surface    : agar.gui.surface.surface_access_t);
  pragma import (c, replace_surface_no_copy, "AG_WidgetReplaceSurfaceNODUP");

  procedure unmap_surface
    (widget     : widget_access_t;
     surface_id : surface_id_t);
  pragma import (c, unmap_surface, "agar_widget_unmap_surface");

  procedure update_surface
    (widget     : widget_access_t;
     surface_id : surface_id_t);
  pragma import (c, update_surface, "agar_widget_update_surface");

  procedure blit
    (widget  : widget_access_t;
     surface : agar.gui.surface.surface_access_t;
     x       : natural;
     y       : natural);
  pragma inline (blit);

  procedure blit_from
    (dest_widget : widget_access_t;
     src_widget  : widget_access_t;
     surface_id  : surface_id_t;
     rect        : agar.gui.rect.rect_access_t;
     x           : integer;
     y           : integer);
  pragma inline (blit_from);

  procedure blit_surface
    (widget     : widget_access_t;
     surface_id : surface_id_t;
     x          : integer;
     y          : integer);
  pragma inline (blit_surface); 

  -- rendering

  procedure push_clip_rect
    (widget : widget_access_t;
     rect   : agar.gui.rect.rect_t);
  pragma import (c, push_clip_rect, "AG_PushClipRect");

  procedure pop_clip_rect (widget : widget_access_t);
  pragma import (c, pop_clip_rect, "AG_PopClipRect");

  -- missing: push_cursor - documented but apparently not implemented
  -- missing: pop_cursor
  
  procedure put_pixel
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     color  : agar.core.types.color_t);
  pragma inline (put_pixel);

  procedure put_pixel32
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     color  : agar.core.types.uint32_t);
  pragma inline (put_pixel32);
 
  procedure put_pixel_rgb
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     r      : agar.core.types.uint8_t;
     g      : agar.core.types.uint8_t;
     b      : agar.core.types.uint8_t); 
  pragma inline (put_pixel_rgb);

  procedure blend_pixel_rgba
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     color  : color_t;
     func   : agar.gui.colors.blend_func_t);
  pragma inline (blend_pixel_rgba);

  procedure blend_pixel_32
    (widget : widget_access_t;
     x      : natural;
     y      : natural;
     color  : agar.core.types.uint32_t;
     func   : agar.gui.colors.blend_func_t);
  pragma inline (blend_pixel_32);

  -- misc

  function find_point
    (class_mask : string;
     x          : natural;
     y          : natural) return widget_access_t;
  pragma inline (find_point);
 
  function find_rect
    (class_mask : string;
     x          : natural;
     y          : natural;
     w          : positive;
     h          : positive) return widget_access_t;
  pragma inline (find_rect);

  -- coordinates

  function absolute_coords_inside
    (widget : widget_access_t;
     x      : natural;
     y      : natural) return boolean;
  pragma inline (absolute_coords_inside);

  function relative_coords_inside
    (widget : widget_access_t;
     x      : natural;
     y      : natural) return boolean;
  pragma inline (relative_coords_inside);

  -- position and size setting

  procedure set_position
    (widget : widget_access_t;
     x      : natural;
     y      : natural);
  pragma inline (set_position);

  procedure modify_position
    (widget : widget_access_t;
     x      : integer := 0;
     y      : integer := 0);
  pragma inline (modify_position);

  procedure set_size
    (widget : widget_access_t;
     width  : positive;
     height : positive);
  pragma inline (set_size);

  procedure modify_size
    (widget : widget_access_t;
     width  : integer := 0;
     height : integer := 0);
  pragma inline (modify_size);

  -- 'casting'

  function object (widget : widget_access_t) return agar.core.object.object_access_t;
  pragma inline (object);

  -- bindings

  package bindings is

    procedure bind_pointer
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.void_ptr_t);
    pragma inline (bind_pointer);

    procedure bind_property
      (widget  : widget_access_t;
       binding : string;
       object  : agar.core.object.object_access_t;
       name    : string);
    pragma inline (bind_property);

    procedure bind_boolean
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.boolean_access_t);
    pragma inline (bind_boolean);

    procedure bind_integer
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.integer_access_t);
    pragma inline (bind_integer);

    procedure bind_unsigned
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.unsigned_access_t);
    pragma inline (bind_unsigned);

    procedure bind_float
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.float_access_t);
    pragma inline (bind_float);

    procedure bind_double
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.double_access_t);
    pragma inline (bind_double);

    procedure bind_uint8
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint8_ptr_t);
    pragma inline (bind_uint8);

    procedure bind_int8
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.int8_ptr_t);
    pragma inline (bind_int8);

    procedure bind_flag8
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint8_ptr_t;
       mask     : agar.core.types.uint8_t);
    pragma inline (bind_flag8);

    procedure bind_uint16
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint16_ptr_t);
    pragma inline (bind_uint16);

    procedure bind_int16
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.int16_ptr_t);
    pragma inline (bind_int16);

    procedure bind_flag16
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint16_ptr_t;
       mask     : agar.core.types.uint16_t);
    pragma inline (bind_flag16);

    procedure bind_uint32
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint32_ptr_t);
    pragma inline (bind_uint32);

    procedure bind_int32
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.int32_ptr_t);
    pragma inline (bind_int32);

    procedure bind_flag32
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint32_ptr_t;
       mask     : agar.core.types.uint32_t);
    pragma inline (bind_flag32);

    -- get

    function get_pointer
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.void_ptr_t;
    pragma inline (get_pointer);

    function get_boolean
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.boolean_t;
    pragma inline (get_boolean);

    function get_integer
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.integer_t;
    pragma inline (get_integer);

    function get_unsigned
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.unsigned_t;
    pragma inline (get_unsigned);

    function get_float
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.float_t;
    pragma inline (get_float);

    function get_double
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.double_t;
    pragma inline (get_double);

    function get_uint8
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.uint8_t;
    pragma inline (get_uint8);

    function get_int8
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.int8_t;
    pragma inline (get_int8);

    function get_uint16
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.uint16_t;
    pragma inline (get_uint16);

    function get_int16
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.int16_t;
    pragma inline (get_int16);

    function get_uint32
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.uint32_t;
    pragma inline (get_uint32);

    function get_int32
      (widget   : widget_access_t;
       binding  : string) return agar.core.types.int32_t;
    pragma inline (get_int32);

    -- set

    procedure set_pointer
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.void_ptr_t);
    pragma inline (set_pointer);

    procedure set_boolean
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.boolean_t);
    pragma inline (set_boolean);

    procedure set_integer
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.integer_t);
    pragma inline (set_integer);

    procedure set_unsigned
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.unsigned_t);
    pragma inline (set_unsigned);

    procedure set_float
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.float_t);
    pragma inline (set_float);

    procedure set_double
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.double_t);
    pragma inline (set_double);

    procedure set_uint8
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint8_t);
    pragma inline (set_uint8);

    procedure set_int8
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.int8_t);
    pragma inline (set_int8);

    procedure set_uint16
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint16_t);
    pragma inline (set_uint16);

    procedure set_int16
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.int16_t);
    pragma inline (set_int16);

    procedure set_uint32
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.uint32_t);
    pragma inline (set_uint32);

    procedure set_int32
      (widget   : widget_access_t;
       binding  : string;
       variable : agar.core.types.int32_t);
    pragma inline (set_int32);

  end bindings;

private

  -- widget type
  type widget_private_t is record
    r_view           : agar.gui.rect.rect2_t;
    r_sens           : agar.gui.rect.rect2_t;

    surfaces         : access agar.gui.surface.surface_access_t;
    surface_flags    : access c.unsigned;
    nsurfaces        : c.unsigned;

    -- openGL
    textures         : access c.unsigned;
    texcoords        : access c.c_float;
    texture_gc       : access c.unsigned;
    ntextures_gc     : c.unsigned;

    bindings_lock    : agar.core.threads.mutex_t;
    bindings         : binding_slist.head_t;
    menus            : menu_slist.head_t;
  end record;
  pragma convention (c, widget_private_t);

end agar.gui.widget;
