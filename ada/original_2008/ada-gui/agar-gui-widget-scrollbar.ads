with agar.core.event;
with agar.core.timeout;

package agar.gui.widget.scrollbar is

  use type c.unsigned;

  type type_t is (SCROLLBAR_HORIZ, SCROLLBAR_VERT);
   for type_t use (SCROLLBAR_HORIZ => 0, SCROLLBAR_VERT => 1);
   for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  type button_t is (
    SCROLLBAR_BUTTON_NONE,
    SCROLLBAR_BUTTON_DEC,
    SCROLLBAR_BUTTON_INC,
    SCROLLBAR_BUTTON_SCROLL
  );
  for button_t use (
    SCROLLBAR_BUTTON_NONE   => 0,
    SCROLLBAR_BUTTON_DEC    => 1,
    SCROLLBAR_BUTTON_INC    => 2,
    SCROLLBAR_BUTTON_SCROLL => 3
  );
  for button_t'size use c.unsigned'size;
  pragma convention (c, button_t);

  type flags_t is new c.unsigned;
  SCROLLBAR_HFILL     : constant flags_t := 16#01#;
  SCROLLBAR_VFILL     : constant flags_t := 16#02#;
  SCROLLBAR_EXPAND    : constant flags_t := SCROLLBAR_HFILL or SCROLLBAR_VFILL;

  type scrollbar_t is limited private;
  type scrollbar_access_t is access all scrollbar_t;
  pragma convention (c, scrollbar_access_t);

  -- API

  function allocate_integer
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     min      : agar.core.types.integer_access_t;
     max      : agar.core.types.integer_access_t;
     visible  : agar.core.types.integer_access_t) return scrollbar_access_t;
  pragma import (c, allocate_integer, "AG_ScrollbarNewInt");
 
  function allocate_unsigned
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     min      : agar.core.types.unsigned_access_t;
     max      : agar.core.types.unsigned_access_t;
     visible  : agar.core.types.unsigned_access_t) return scrollbar_access_t;
  pragma import (c, allocate_unsigned, "AG_ScrollbarNewUint");

  function allocate_float
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     min      : agar.core.types.float_access_t;
     max      : agar.core.types.float_access_t;
     visible  : agar.core.types.float_access_t) return scrollbar_access_t;
  pragma import (c, allocate_float, "AG_ScrollbarNewFloat");

  function allocate_double
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     min      : agar.core.types.double_access_t;
     max      : agar.core.types.double_access_t;
     visible  : agar.core.types.double_access_t) return scrollbar_access_t;
  pragma import (c, allocate_double, "AG_ScrollbarNewDouble");

  function allocate_uint8
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.uint8_ptr_t;
     min      : agar.core.types.uint8_ptr_t;
     max      : agar.core.types.uint8_ptr_t;
     visible  : agar.core.types.uint8_ptr_t) return scrollbar_access_t;
  pragma import (c, allocate_uint8, "AG_ScrollbarNewUint8");

  function allocate_int8
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.int8_ptr_t;
     min      : agar.core.types.int8_ptr_t;
     max      : agar.core.types.int8_ptr_t;
     visible  : agar.core.types.int8_ptr_t) return scrollbar_access_t;
  pragma import (c, allocate_int8, "AG_ScrollbarNewSint8");

  function allocate_uint16
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.uint16_ptr_t;
     min      : agar.core.types.uint16_ptr_t;
     max      : agar.core.types.uint16_ptr_t;
     visible  : agar.core.types.uint16_ptr_t) return scrollbar_access_t;
  pragma import (c, allocate_uint16, "AG_ScrollbarNewUint16");

  function allocate_int16
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.int16_ptr_t;
     min      : agar.core.types.int16_ptr_t;
     max      : agar.core.types.int16_ptr_t;
     visible  : agar.core.types.int16_ptr_t) return scrollbar_access_t;
  pragma import (c, allocate_int16, "AG_ScrollbarNewSint16");

  function allocate_uint32
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.uint32_ptr_t;
     min      : agar.core.types.uint32_ptr_t;
     max      : agar.core.types.uint32_ptr_t;
     visible  : agar.core.types.uint32_ptr_t) return scrollbar_access_t;
  pragma import (c, allocate_uint32, "AG_ScrollbarNewUint32");

  function allocate_int32
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.int32_ptr_t;
     min      : agar.core.types.int32_ptr_t;
     max      : agar.core.types.int32_ptr_t;
     visible  : agar.core.types.int32_ptr_t) return scrollbar_access_t;
  pragma import (c, allocate_int32, "AG_ScrollbarNewSint32");

  procedure set_size
    (scrollbar : scrollbar_access_t;
     size      : natural);
  pragma inline (set_size);

  function get_size (scrollbar : scrollbar_access_t) return natural;
  pragma inline (get_size);

  function visible (scrollbar : scrollbar_access_t) return boolean;
  pragma inline (visible);

  procedure set_increment
    (scrollbar : scrollbar_access_t;
     increment : positive);
  pragma inline (set_increment);

  procedure set_increment
    (scrollbar : scrollbar_access_t;
     increment : long_float);
  pragma inline (set_increment);

  function widget (scrollbar : scrollbar_access_t) return widget_access_t;
  pragma inline (widget);

private

  type scrollbar_t is record
    widget          : aliased widget_t;
    flags           : flags_t;

    value           : c.int;
    min             : c.int;
    max             : c.int;
    visible         : c.int;
    bar_type        : type_t;
    bar_button      : button_t;
    button_width    : c.int;
    bar_width       : c.int;
    arrow_height    : c.int;
    button_inc_func : access agar.core.event.event_t;
    button_dec_func : access agar.core.event.event_t;
    scroll_to       : agar.core.timeout.timeout_t;
    inc_to          : agar.core.timeout.timeout_t;
    dec_to          : agar.core.timeout.timeout_t;
    x_offset        : c.int;
    extent          : c.int;
    r_inc           : c.double;
    i_inc           : c.int;
  end record;
  pragma convention (c, scrollbar_t);

end agar.gui.widget.scrollbar;
