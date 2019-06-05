#!/usr/bin/env lua

local ts = tostring

io.write ([[
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

]])

for _, value in pairs ({8, 16, 32, 64}) do
  io.write ([[
  function allocate_uint]] .. ts (value) .. [[

    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.uint]] .. ts (value) ..[[_access_t;
     min      : agar.core.types.uint]] .. ts (value) ..[[_access_t;
     max      : agar.core.types.uint]] .. ts (value) ..[[_access_t;
     visible  : agar.core.types.uint]] .. ts (value) ..[[_access_t) return scrollbar_access_t;
  pragma import (c, allocate_uint]] .. ts (value) .. [[, "AG_ScrollbarNewUint]] .. ts (value) .. [[");

  function allocate_int]] .. ts (value) .. [[

    (parent   : widget_access_t;
     bar_type : type_t;
     flags    : flags_t;
     value    : agar.core.types.int]] .. ts (value) ..[[_access_t;
     min      : agar.core.types.int]] .. ts (value) ..[[_access_t;
     max      : agar.core.types.int]] .. ts (value) ..[[_access_t;
     visible  : agar.core.types.int]] .. ts (value) ..[[_access_t) return scrollbar_access_t;
  pragma import (c, allocate_int]] .. ts (value) .. [[, "AG_ScrollbarNewSint]] .. ts (value) .. [[");

]])
end
