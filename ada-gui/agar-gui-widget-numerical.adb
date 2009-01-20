package body agar.gui.widget.numerical is

  use type c.int;

  package cbinds is
    function allocate
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr) return numerical_access_t;
    pragma import (c, allocate, "AG_NumericalNew");

    function allocate_float
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : float_access_t) return numerical_access_t;
    pragma import (c, allocate_float, "AG_NumericalNewFlt");

    function allocate_float_ranged
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : float_access_t;
       min    : float_t;
       max    : float_t) return numerical_access_t;
    pragma import (c, allocate_float_ranged, "AG_NumericalNewFltR");

    function allocate_double
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : double_access_t) return numerical_access_t;
    pragma import (c, allocate_double, "AG_NumericalNewDbl");

    function allocate_double_ranged
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : double_access_t;
       min    : double_t;
       max    : double_t) return numerical_access_t;
    pragma import (c, allocate_double_ranged, "AG_NumericalNewDblR");

    function allocate_integer
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : integer_access_t) return numerical_access_t;
    pragma import (c, allocate_integer, "AG_NumericalNewInt");

    function allocate_integer_ranged
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : integer_access_t;
       min    : integer_t;
       max    : integer_t) return numerical_access_t;
    pragma import (c, allocate_integer_ranged, "AG_NumericalNewIntR");

    function allocate_unsigned
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : unsigned_access_t) return numerical_access_t;
    pragma import (c, allocate_unsigned, "AG_NumericalNewUint");

    function allocate_unsigned_ranged
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : unsigned_access_t;
       min    : unsigned_t;
       max    : unsigned_t) return numerical_access_t;
    pragma import (c, allocate_unsigned_ranged, "AG_NumericalNewUintR");

    function allocate_uint8
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : agar.core.types.uint8_ptr_t) return numerical_access_t;
    pragma import (c, allocate_uint8, "AG_NumericalNewUint8");

    function allocate_uint16
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : agar.core.types.uint16_ptr_t) return numerical_access_t;
    pragma import (c, allocate_uint16, "AG_NumericalNewUint16");

    function allocate_uint32
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : agar.core.types.uint32_ptr_t) return numerical_access_t;
    pragma import (c, allocate_uint32, "AG_NumericalNewUint32");

    function allocate_int8
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : agar.core.types.int8_ptr_t) return numerical_access_t;
    pragma import (c, allocate_int8, "AG_NumericalNewSint8");

    function allocate_int16
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : agar.core.types.int16_ptr_t) return numerical_access_t;
    pragma import (c, allocate_int16, "AG_NumericalNewSint16");

    function allocate_int32
      (parent : widget_access_t;
       flags  : flags_t;
       unit   : cs.chars_ptr;
       label  : cs.chars_ptr;
       value  : agar.core.types.int32_ptr_t) return numerical_access_t;
    pragma import (c, allocate_int32, "AG_NumericalNewSint32");

    function set_unit_system
      (numerical : numerical_access_t;
       unit      : cs.chars_ptr) return c.int;
    pragma import (c, set_unit_system, "AG_NumericalSetUnitSystem");

    procedure select_unit
      (numerical : numerical_access_t;
       unit      : cs.chars_ptr);
    pragma import (c, select_unit, "AG_NumericalSelectUnit");

    procedure set_precision
      (numerical : numerical_access_t;
       format    : cs.chars_ptr;
       precision : c.int);
    pragma import (c, set_precision, "AG_NumericalSetPrecision");

    procedure set_writeable
      (numerical : numerical_access_t;
       writeable : c.int);
    pragma import (c, set_writeable, "AG_NumericalSetWriteable");
  end cbinds;

  function allocate
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access));
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access));
      end;
    end if;
  end allocate;

  function allocate_float
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : float_access_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_float
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_float
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value);
      end;
    end if;
  end allocate_float;

  function allocate_float_ranged
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : float_access_t;
     min    : float_t;
     max    : float_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_float_ranged
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value,
         min    => min,
         max    => max);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_float_ranged
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value,
           min    => min,
           max    => max);
      end;
    end if;
  end allocate_float_ranged;

  function allocate_double
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : double_access_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_double
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_double
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value);
      end;
    end if;
  end allocate_double;

  function allocate_double_ranged
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : double_access_t;
     min    : double_t;
     max    : double_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_double_ranged
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value,
         min    => min,
         max    => max);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_double_ranged
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value,
           min    => min,
           max    => max);
      end;
    end if;
  end allocate_double_ranged;

  function allocate_integer
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : integer_access_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_integer
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_integer
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value);
      end;
    end if;
  end allocate_integer;

  function allocate_integer_ranged
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : integer_access_t;
     min    : integer_t;
     max    : integer_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_integer_ranged
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value,
         min    => min,
         max    => max);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_integer_ranged
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value,
           min    => min,
           max    => max);
      end;
    end if;
  end allocate_integer_ranged;

  function allocate_unsigned
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : unsigned_access_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_unsigned
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_unsigned
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value);
      end;
    end if;
  end allocate_unsigned;

  function allocate_unsigned_ranged
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : unsigned_access_t;
     min    : unsigned_t;
     max    : unsigned_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_unsigned_ranged
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value,
         min    => min,
         max    => max);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_unsigned_ranged
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value,
           min    => min,
           max    => max);
      end;
    end if;
  end allocate_unsigned_ranged;

  function allocate_uint8
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : agar.core.types.uint8_ptr_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_uint8
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_uint8
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value);
      end;
    end if;
  end allocate_uint8;

  function allocate_uint16
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : agar.core.types.uint16_ptr_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_uint16
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_uint16
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value);
      end;
    end if;
  end allocate_uint16;

  function allocate_uint32
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : agar.core.types.uint32_ptr_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_uint32
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_uint32
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value);
      end;
    end if;
  end allocate_uint32;

  function allocate_int8
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : agar.core.types.int8_ptr_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_int8
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_int8
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value);
      end;
    end if;
  end allocate_int8;

  function allocate_int16
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : agar.core.types.int16_ptr_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_int16
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_int16
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value);
      end;
    end if;
  end allocate_int16;

  function allocate_int32
    (parent : widget_access_t;
     flags  : flags_t;
     unit   : string;
     label  : string;
     value  : agar.core.types.int32_ptr_t) return numerical_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    if unit /= no_unit then
      return cbinds.allocate_int32
        (parent => parent,
         flags  => flags,
         unit   => cs.null_ptr,
         label  => cs.to_chars_ptr (c_label'unchecked_access),
         value  => value);
    else
      declare
        c_unit : aliased c.char_array := c.to_c (unit);
      begin
        return cbinds.allocate_int32
          (parent => parent,
           flags  => flags,
           unit   => cs.to_chars_ptr (c_unit'unchecked_access),
           label  => cs.to_chars_ptr (c_label'unchecked_access),
           value  => value);
      end;
    end if;
  end allocate_int32;

  function set_unit_system
    (numerical : numerical_access_t;
     unit      : string) return boolean
  is
    c_unit : aliased c.char_array := c.to_c (unit);
  begin
    return cbinds.set_unit_system
      (numerical => numerical,
       unit      => cs.to_chars_ptr (c_unit'unchecked_access)) = 0;
  end set_unit_system;

  procedure select_unit
    (numerical : numerical_access_t;
     unit      : string)
  is
    c_unit : aliased c.char_array := c.to_c (unit);
  begin
    cbinds.select_unit
      (numerical => numerical,
       unit      => cs.to_chars_ptr (c_unit'unchecked_access));
  end select_unit;

  procedure set_precision
    (numerical : numerical_access_t;
     format    : string;
     precision : positive)
  is
    c_format : aliased c.char_array := c.to_c (format);
  begin
    cbinds.set_precision
      (numerical => numerical,
       format    => cs.to_chars_ptr (c_format'unchecked_access),
       precision => c.int (precision));
  end set_precision;

  procedure set_writeable
    (numerical : numerical_access_t;
     writeable : boolean) is
  begin
    if writeable then
      cbinds.set_writeable (numerical, 1);
    else
      cbinds.set_writeable (numerical, 0);
    end if;
  end set_writeable;

  procedure sub_value
    (numerical : numerical_access_t;
     value     : double_t) is
  begin
    add_value (numerical, 0.0 - value);
  end sub_value;

  function widget (numerical : numerical_access_t) return widget_access_t is
  begin
    return numerical.widget'access;
  end widget;

end agar.gui.widget.numerical;
