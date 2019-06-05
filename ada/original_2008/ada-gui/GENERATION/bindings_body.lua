#!/usr/bin/env lua

local ts = tostring

io.write ([[
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
]])

for _, typename in pairs ({"boolean", "integer", "unsigned", "float", "double"}) do
  io.write ([[
    procedure bind_]]..typename..[[

      (widget   : widget_access_t;
       binding  : cs.chars_ptr;
       variable : agar.core.types.]]..typename..[[_access_t);
    pragma import (c, bind_]]..typename..[[, "agar_gui_widget_bind_]]..typename..[[");

]])
end

for _, value in pairs ({8, 16, 32, 64}) do
  io.write ([[
    procedure bind_uint]]..ts (value)..[[

      (widget   : widget_access_t;
       binding  : cs.chars_ptr;
       variable : agar.core.types.uint]]..ts (value)..[[_ptr_t);
    pragma import (c, bind_uint]]..ts (value)..[[, "agar_gui_widget_bind_uint]]..ts (value)..[[");

    procedure bind_int]]..ts (value)..[[

      (widget   : widget_access_t;
       binding  : cs.chars_ptr;
       variable : agar.core.types.int]]..ts (value)..[[_ptr_t);
    pragma import (c, bind_int]]..ts (value)..[[, "agar_gui_widget_bind_int]]..ts (value)..[[");

    procedure bind_flag]]..ts (value)..[[

      (widget   : widget_access_t;
       binding  : cs.chars_ptr;
       variable : agar.core.types.uint]]..ts (value)..[[_ptr_t;
       mask     : agar.core.types.uint]]..ts (value)..[[_t);
    pragma import (c, bind_flag]]..ts (value)..[[, "agar_gui_widget_bind_flag]]..ts (value)..[[");

]])
end

-- get

io.write ([[
    -- get

    function get_pointer
      (widget   : widget_access_t;
       binding  : cs.chars_ptr) return agar.core.types.void_ptr_t;
    pragma import (c, get_pointer, "agar_gui_widget_get_pointer");

]])


for _, typename in pairs ({"boolean", "integer", "unsigned", "float", "double"}) do
  io.write ([[
    function get_]]..typename..[[

      (widget   : widget_access_t;
       binding  : cs.chars_ptr) return agar.core.types.]]..typename..[[_t;
    pragma import (c, get_]]..typename..[[, "agar_gui_widget_get_]]..typename..[[");

]])
end

for _, value in pairs ({8, 16, 32, 64}) do
  io.write ([[
    function get_uint]]..ts (value)..[[

      (widget   : widget_access_t;
       binding  : cs.chars_ptr) return agar.core.types.uint]]..ts (value)..[[_t;
    pragma import (c, get_uint]]..ts (value)..[[, "agar_gui_widget_get_uint]]..ts (value)..[[");

    function get_int]]..ts (value)..[[

      (widget   : widget_access_t;
       binding  : cs.chars_ptr) return agar.core.types.int]]..ts (value)..[[_t;
    pragma import (c, get_int]]..ts (value)..[[, "agar_gui_widget_get_int]]..ts (value)..[[");

]])
end

-- set

io.write ([[
    -- set

    procedure set_pointer
      (widget   : widget_access_t;
       binding  : cs.chars_ptr;
       variable : agar.core.types.void_ptr_t);
    pragma import (c, set_pointer, "agar_gui_widget_set_pointer");

]])

for _, typename in pairs ({"boolean", "integer", "unsigned", "float", "double"}) do
  io.write ([[ 
    procedure set_]]..typename..[[

      (widget   : widget_access_t;
       binding  : cs.chars_ptr;
       variable : agar.core.types.]]..typename..[[_t);
    pragma import (c, set_]]..typename..[[, "agar_gui_widget_set_]]..typename..[[");

]])
end

for _, value in pairs ({8, 16, 32, 64}) do
  io.write ([[
    procedure set_uint]]..ts (value)..[[

      (widget   : widget_access_t;
       binding  : cs.chars_ptr;
       variable : agar.core.types.uint]]..ts (value)..[[_t);
    pragma import (c, set_uint]]..ts (value)..[[, "agar_gui_widget_set_uint]]..ts(value)..[[");

    procedure set_int]]..ts (value)..[[

      (widget   : widget_access_t;
       binding  : cs.chars_ptr;
       variable : agar.core.types.int]]..ts (value)..[[_t);
    pragma import (c, set_int]]..ts (value)..[[, "agar_gui_widget_set_int]]..ts(value)..[[");

]])
end

io.write ([[
  end cbinds;
]])

-- bind

io.write ([[

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

]])

for _, typename in pairs ({"boolean", "integer", "unsigned", "float", "double"}) do
  io.write ([[
  procedure bind_]]..typename..[[

    (widget   : widget_access_t;
     binding  : string;
     variable : agar.core.types.]]..typename..[[_access_t)
  is
    c_binding : aliased c.char_array := c.to_c (binding);
  begin
    cbinds.bind_]]..typename..[[

      (widget   => widget,
       binding  => cs.to_chars_ptr (c_binding'unchecked_access),
       variable => variable);
  end bind_]]..typename..[[;

]])
end

for _, value in pairs ({8, 16, 32, 64}) do
  io.write ([[
  procedure bind_uint]]..ts (value)..[[

    (widget   : widget_access_t;
     binding  : string;
     variable : agar.core.types.uint]]..ts (value)..[[_ptr_t)
  is
    c_binding : aliased c.char_array := c.to_c (binding);
  begin
    cbinds.bind_uint]]..ts (value)..[[

      (widget   => widget,
       binding  => cs.to_chars_ptr (c_binding'unchecked_access),
       variable => variable);
  end bind_uint]]..ts (value)..[[;

  procedure bind_int]]..ts (value)..[[

    (widget   : widget_access_t;
     binding  : string;
     variable : agar.core.types.int]]..ts (value)..[[_ptr_t)
  is
    c_binding : aliased c.char_array := c.to_c (binding);
  begin
    cbinds.bind_int]]..ts (value)..[[

      (widget   => widget,
       binding  => cs.to_chars_ptr (c_binding'unchecked_access),
       variable => variable);
  end bind_int]]..ts (value)..[[;

  procedure bind_flag]]..ts (value)..[[

    (widget   : widget_access_t;
     binding  : string;
     variable : agar.core.types.uint]]..ts (value)..[[_ptr_t;
     mask     : agar.core.types.uint]]..ts (value)..[[_t)
  is
    c_binding : aliased c.char_array := c.to_c (binding);
  begin
    cbinds.bind_flag]]..ts (value)..[[

      (widget   => widget,
       binding  => cs.to_chars_ptr (c_binding'unchecked_access),
       variable => variable,
       mask     => mask);
  end bind_flag]]..ts (value)..[[;

]])
end

-- get

io.write ([[
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

]])

for _, typename in pairs ({"boolean", "integer", "unsigned", "float", "double"}) do
  io.write ([[
  function get_]]..typename..[[

    (widget   : widget_access_t;
     binding  : string) return agar.core.types.]]..typename..[[_t
  is
    c_binding : aliased c.char_array := c.to_c (binding);
  begin
    return cbinds.get_]]..typename..[[

      (widget  => widget,
       binding => cs.to_chars_ptr (c_binding'unchecked_access));
  end get_]]..typename..[[;

]])
end

for _, value in pairs ({8, 16, 32, 64}) do
  io.write ([[
  function get_uint]]..ts (value)..[[

    (widget  : widget_access_t;
     binding : string) return agar.core.types.uint]]..ts (value)..[[_t
  is
    c_binding : aliased c.char_array := c.to_c (binding);
  begin
    return cbinds.get_uint]]..ts (value)..[[

      (widget  => widget,
       binding => cs.to_chars_ptr (c_binding'unchecked_access));
  end get_uint]]..ts (value)..[[;

  function get_int]]..ts (value)..[[

    (widget  : widget_access_t;
     binding : string) return agar.core.types.int]]..ts (value)..[[_t
  is
    c_binding : aliased c.char_array := c.to_c (binding);
  begin
    return cbinds.get_int]]..ts (value)..[[

      (widget  => widget,
       binding => cs.to_chars_ptr (c_binding'unchecked_access));
  end get_int]]..ts (value)..[[;

]])
end

-- set

io.write ([[
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

]])

for _, typename in pairs ({"boolean", "integer", "unsigned", "float", "double"}) do
  io.write ([[
  procedure set_]]..typename..[[

    (widget   : widget_access_t;
     binding  : string;
     variable : agar.core.types.]]..typename..[[_t)
  is
    c_binding : aliased c.char_array := c.to_c (binding);
  begin
    cbinds.set_]]..typename..[[

      (widget   => widget,
       binding  => cs.to_chars_ptr (c_binding'unchecked_access),
       variable => variable);
  end set_]]..typename..[[;

]])
end

for _, value in pairs ({8, 16, 32, 64}) do
  io.write ([[
  procedure set_uint]]..ts (value)..[[

    (widget   : widget_access_t;
     binding  : string;
     variable : agar.core.types.uint]]..ts (value)..[[_t)
  is
    c_binding : aliased c.char_array := c.to_c (binding);
  begin
    cbinds.set_uint]]..ts (value)..[[

      (widget   => widget,
       binding  => cs.to_chars_ptr (c_binding'unchecked_access),
       variable => variable);
  end set_uint]]..ts (value)..[[;

  procedure set_int]]..ts (value)..[[

    (widget   : widget_access_t;
     binding  : string;
     variable : agar.core.types.int]]..ts (value)..[[_t)
  is
    c_binding : aliased c.char_array := c.to_c (binding);
  begin
    cbinds.set_int]]..ts (value)..[[

      (widget   => widget,
       binding  => cs.to_chars_ptr (c_binding'unchecked_access),
       variable => variable);
  end set_int]]..ts (value)..[[;

]])
end

io.write ([[
end bindings;

]])
