with agar.core.timeout;

package agar.gui.widget.slider is

  use type c.unsigned;

  type flags_t is new c.unsigned;
  SLIDER_HFILL     : constant flags_t := 16#01#;
  SLIDER_VFILL     : constant flags_t := 16#02#;
  SLIDER_FOCUSABLE : constant flags_t := 16#04#;
  SLIDER_EXPAND    : constant flags_t := SLIDER_HFILL or SLIDER_VFILL;

  type type_t is (SLIDER_HORIZ, SLIDER_VERT);
  for type_t use (SLIDER_HORIZ => 0, SLIDER_VERT => 1);
  for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  type button_t is (
    SLIDER_BUTTON_NONE,
    SLIDER_BUTTON_DEC,
    SLIDER_BUTTON_INC,
    SLIDER_BUTTON_SCROLL
  );
  for button_t use (
    SLIDER_BUTTON_NONE   => 0,
    SLIDER_BUTTON_DEC    => 1,
    SLIDER_BUTTON_INC    => 2,
    SLIDER_BUTTON_SCROLL => 3
  );
  for button_t'size use c.unsigned'size;
  pragma convention (c, button_t);

  type slider_t is limited private;
  type slider_access_t is access all slider_t;
  pragma convention (c, slider_access_t);

  -- API

  function allocate_integer
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.integer_access_t;
     min      : agar.core.types.integer_access_t;
     max      : agar.core.types.integer_access_t) return slider_access_t;
  pragma import (c, allocate_integer, "AG_SliderNewInt");
 
  function allocate_unsigned
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.unsigned_access_t;
     min      : agar.core.types.unsigned_access_t;
     max      : agar.core.types.unsigned_access_t) return slider_access_t;
  pragma import (c, allocate_unsigned, "AG_SliderNewUint");

  function allocate_float
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.float_access_t;
     min      : agar.core.types.float_access_t;
     max      : agar.core.types.float_access_t) return slider_access_t;
  pragma import (c, allocate_float, "AG_SliderNewFloat");

  function allocate_double
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.double_access_t;
     min      : agar.core.types.double_access_t;
     max      : agar.core.types.double_access_t) return slider_access_t;
  pragma import (c, allocate_double, "AG_SliderNewDouble");

  function allocate_uint8
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.uint8_ptr_t;
     min      : agar.core.types.uint8_ptr_t;
     max      : agar.core.types.uint8_ptr_t) return slider_access_t;
  pragma import (c, allocate_uint8, "AG_SliderNewUint8");

  function allocate_int8
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.int8_ptr_t;
     min      : agar.core.types.int8_ptr_t;
     max      : agar.core.types.int8_ptr_t) return slider_access_t;
  pragma import (c, allocate_int8, "AG_SliderNewSint8");

  function allocate_uint16
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.uint16_ptr_t;
     min      : agar.core.types.uint16_ptr_t;
     max      : agar.core.types.uint16_ptr_t) return slider_access_t;
  pragma import (c, allocate_uint16, "AG_SliderNewUint16");

  function allocate_int16
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.int16_ptr_t;
     min      : agar.core.types.int16_ptr_t;
     max      : agar.core.types.int16_ptr_t) return slider_access_t;
  pragma import (c, allocate_int16, "AG_SliderNewSint16");

  function allocate_uint32
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.uint32_ptr_t;
     min      : agar.core.types.uint32_ptr_t;
     max      : agar.core.types.uint32_ptr_t) return slider_access_t;
  pragma import (c, allocate_uint32, "AG_SliderNewUint32");

  function allocate_int32
    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.int32_ptr_t;
     min      : agar.core.types.int32_ptr_t;
     max      : agar.core.types.int32_ptr_t) return slider_access_t;
  pragma import (c, allocate_int32, "AG_SliderNewSint32");

  procedure set_increment
    (slider    : slider_access_t;
     increment : positive);
  pragma inline (set_increment);

  procedure set_increment
    (slider    : slider_access_t;
     increment : long_float);
  pragma inline (set_increment);

  function widget (slider : slider_access_t) return widget_access_t;
  pragma inline (widget);

private

  type slider_t is record
    widget         : aliased widget_t;
    flags          : flags_t;
    value          : c.int;
    min            : c.int;
    max            : c.int;
    slider_type    : type_t;
    ctl_pressed    : c.int;
    w_control_pref : c.int;
    w_control      : c.int;
    inc_to         : agar.core.timeout.timeout_t;
    dec_to         : agar.core.timeout.timeout_t;
    x_offset       : c.int;
    extent         : c.int;
    r_inc          : c.double;
    i_inc          : c.int;
  end record;
  pragma convention (c, slider_t);

end agar.gui.widget.slider;
