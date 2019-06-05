with agar.gui.unit;
with agar.gui.widget.button;
with agar.gui.widget.textbox;
with agar.gui.widget.ucombo;

package agar.gui.widget.numerical is

  use type c.unsigned;

  type flags_t is new c.unsigned;
  NUMERICAL_HFILL    : constant flags_t := 16#01#;
  NUMERICAL_VFILL    : constant flags_t := 16#02#;

  type numerical_t is limited private;
  type numerical_access_t is access all numerical_t;
  pragma convention (c, numerical_access_t);

  type float_t is new c.c_float;
  type float_access_t is access all float_t;
  pragma convention (c, float_t);
  pragma convention (c, float_access_t);

  type double_t is new c.double;
  type double_access_t is access all double_t;
  pragma convention (c, double_t);
  pragma convention (c, double_access_t);

  type integer_t is new c.int;
  type integer_access_t is access all integer_t;
  pragma convention (c, integer_t);
  pragma convention (c, integer_access_t);

  type unsigned_t is new c.unsigned;
  type unsigned_access_t is access all unsigned_t;
  pragma convention (c, unsigned_t);
  pragma convention (c, unsigned_access_t);

  -- do not show unit for numerical
  no_unit : constant string := "";

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string) return numerical_access_t;
  pragma inline (allocate);

  function allocate_float
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : float_access_t) return numerical_access_t;
  pragma inline (allocate_float);

  function allocate_float_ranged
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : float_access_t;
     min    : float_t;
     max    : float_t) return numerical_access_t;
  pragma inline (allocate_float_ranged);

  function allocate_double
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : double_access_t) return numerical_access_t;
  pragma inline (allocate_double);

  function allocate_double_ranged
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : double_access_t;
     min    : double_t;
     max    : double_t) return numerical_access_t;
  pragma inline (allocate_double_ranged);

  function allocate_integer
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : integer_access_t) return numerical_access_t;
  pragma inline (allocate_integer);

  function allocate_integer_ranged
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : integer_access_t;
     min    : integer_t;
     max    : integer_t) return numerical_access_t;
  pragma inline (allocate_integer_ranged);

  function allocate_unsigned
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : unsigned_access_t) return numerical_access_t;
  pragma inline (allocate_unsigned);

  function allocate_unsigned_ranged
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : unsigned_access_t;
     min    : unsigned_t;
     max    : unsigned_t) return numerical_access_t;
  pragma inline (allocate_unsigned_ranged);

  function allocate_uint8
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : agar.core.types.uint8_ptr_t) return numerical_access_t;
  pragma inline (allocate_uint8);

  function allocate_uint16
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : agar.core.types.uint16_ptr_t) return numerical_access_t;
  pragma inline (allocate_uint16);

  function allocate_uint32
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : agar.core.types.uint32_ptr_t) return numerical_access_t;
  pragma inline (allocate_uint32);

  function allocate_int8
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : agar.core.types.int8_ptr_t) return numerical_access_t;
  pragma inline (allocate_int8);

  function allocate_int16
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : agar.core.types.int16_ptr_t) return numerical_access_t;
  pragma inline (allocate_int16);

  function allocate_int32
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : agar.core.types.int32_ptr_t) return numerical_access_t;
  pragma inline (allocate_int32);

  procedure set_min
    (numerical : numerical_access_t;
     min_value : double_t);
  pragma import (c, set_min, "AG_NumericalSetMin");

  procedure set_max
    (numerical : numerical_access_t;
     max_value : double_t);
  pragma import (c, set_max, "AG_NumericalSetMax");

  procedure set_range
    (numerical : numerical_access_t;
     min_value : double_t;
     max_value : double_t);
  pragma import (c, set_range, "AG_NumericalSetRange");

  procedure set_increment
    (numerical : numerical_access_t;
     increment : double_t);
  pragma import (c, set_increment, "AG_NumericalSetIncrement");

  function set_unit_system
    (numerical : numerical_access_t;
     unit      : string) return boolean;
  pragma inline (set_unit_system);

  procedure select_unit
    (numerical : numerical_access_t;
     unit      : string);
  pragma inline (select_unit);

  procedure set_precision
    (numerical : numerical_access_t;
     format    : string;
     precision : positive);
  pragma inline (set_precision);

  procedure set_writeable
    (numerical : numerical_access_t;
     writeable : boolean);
  pragma inline (set_writeable);

  -- type-independent value manipulation

  procedure set_value
    (numerical : numerical_access_t;
     value     : double_t);
  pragma import (c, set_value, "AG_NumericalSetValue");

  procedure add_value
    (numerical : numerical_access_t;
     value     : double_t);
  pragma import (c, add_value, "AG_NumericalAddValue");

  procedure sub_value
    (numerical : numerical_access_t;
     value     : double_t);
  pragma inline (sub_value);

  function widget (numerical : numerical_access_t) return widget_access_t;
  pragma inline (widget);

private

  type format_t is array (1 .. 32) of aliased c.char;
  pragma convention (c, format_t);

  type numerical_t is record
    widget     : aliased widget_t;
    flags      : flags_t;
    value      : c.double;
    min        : c.double;
    max        : c.double;
    inc        : c.double;
    format     : format_t;
    unit       : agar.gui.unit.unit_access_t;
    writeable  : c.int;
    input      : agar.gui.widget.textbox.textbox_access_t;
    units      : agar.gui.widget.ucombo.ucombo_access_t;
    inc_bu     : agar.gui.widget.button.button_access_t;
    dec_bu     : agar.gui.widget.button.button_access_t;
    w_unit_sel : c.int;
    h_unit_sel : c.int;
    w_pre_unit : c.int;
  end record;
  pragma convention (c, numerical_t);

end agar.gui.widget.numerical;
