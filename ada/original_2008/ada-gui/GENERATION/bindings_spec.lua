#!/usr/bin/env lua

local ts = tostring

io.write ([[
package bindings is

]])

-- bind

io.write ([[
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

]])

for _, value in pairs ({8, 16, 32, 64}) do
  io.write ([[
  procedure bind_uint]] .. ts (value) .. [[

    (widget   : widget_access_t;
     binding  : string;
     variable : agar.core.types.uint]] .. ts (value) ..[[_ptr_t);
  pragma inline (bind_uint]] .. ts (value) .. [[);

  procedure bind_int]] .. ts (value) .. [[

    (widget   : widget_access_t;
     binding  : string;
     variable : agar.core.types.int]] .. ts (value) ..[[_ptr_t);
  pragma inline (bind_int]] .. ts (value) .. [[);

  procedure bind_flag]] .. ts (value) .. [[

    (widget   : widget_access_t;
     binding  : string;
     variable : agar.core.types.uint]] .. ts (value) ..[[_ptr_t;
     mask     : agar.core.types.uint]] .. ts (value) ..[[_t);
  pragma inline (bind_flag]] .. ts (value) .. [[);

]])
end

-- get

io.write ([[
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

]])

for _, value in pairs ({8, 16, 32, 64}) do
  io.write ([[
  function get_uint]] .. ts (value) .. [[

    (widget   : widget_access_t;
     binding  : string) return agar.core.types.uint]] .. ts (value) ..[[_t;
  pragma inline (get_uint]] .. ts (value) .. [[);

  function get_int]] .. ts (value) .. [[

    (widget   : widget_access_t;
     binding  : string) return agar.core.types.int]] .. ts (value) ..[[_t;
  pragma inline (get_int]] .. ts (value) .. [[);

]])
end

-- set

io.write ([[
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

]])

for _, value in pairs ({8, 16, 32, 64}) do
  io.write ([[
  procedure set_uint]] .. ts (value) .. [[

    (widget   : widget_access_t;
     binding  : string;
     variable : agar.core.types.uint]] .. ts (value) ..[[_t);
  pragma inline (set_uint]] .. ts (value) .. [[);

  procedure set_int]] .. ts (value) .. [[

    (widget   : widget_access_t;
     binding  : string;
     variable : agar.core.types.int]] .. ts (value) ..[[_t);
  pragma inline (set_int]] .. ts (value) .. [[);

]])
end

io.write ([[
end bindings;

]])
