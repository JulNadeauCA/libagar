package body agar.gui.widget.checkbox is

  package cbinds is
    function allocate
      (parent : widget_access_t;
       flags  : flags_t;
       label  : cs.chars_ptr) return checkbox_access_t;
    pragma import (c, allocate, "AG_CheckboxNewS");
  
    function allocate_function
      (parent : widget_access_t;
       flags  : flags_t;
       label  : cs.chars_ptr;
       func   : agar.core.event.callback_t;
       fmt    : agar.core.types.void_ptr_t) return checkbox_access_t;
    pragma import (c, allocate_function, "AG_CheckboxNewFn");
  
    function allocate_integer
      (parent : widget_access_t;
       flags  : flags_t;
       label  : cs.chars_ptr;
       ptr    : access c.int) return checkbox_access_t;
    pragma import (c, allocate_integer, "AG_CheckboxNewInt");
  
    function allocate_flags
      (parent : widget_access_t;
       flags  : flags_t;
       label  : cs.chars_ptr;
       ptr    : access c.unsigned;
       mask   : c.unsigned) return checkbox_access_t;
    pragma import (c, allocate_flags, "AG_CheckboxNewFlag");
  
    function allocate_flags32
      (parent : widget_access_t;
       flags  : flags_t;
       label  : cs.chars_ptr;
       ptr    : access agar.core.types.uint32_t;
       mask   : agar.core.types.uint32_t) return checkbox_access_t;
    pragma import (c, allocate_flags32, "AG_CheckboxNewFlag32");
  end cbinds;

  --

  function allocate
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string) return checkbox_access_t
  is
    ca_label : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (ca_label'unchecked_access));
  end allocate;

  function allocate_function
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string;
     func   : agar.core.event.callback_t) return checkbox_access_t
  is
    ca_label : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate_function
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (ca_label'unchecked_access),
       func   => func,
       fmt    => agar.core.types.null_ptr);
  end allocate_function;

  function allocate_integer
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string;
     ptr    : access c.int) return checkbox_access_t
  is
    ca_label : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate_integer
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (ca_label'unchecked_access),
       ptr    => ptr);
  end allocate_integer;

  function allocate_flags
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string;
     ptr    : access c.unsigned;
     mask   : c.unsigned) return checkbox_access_t
  is
    ca_label : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate_flags
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (ca_label'unchecked_access),
       ptr    => ptr,
       mask   => mask);
  end allocate_flags;

  function allocate_flags32
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string;
     ptr    : access agar.core.types.uint32_t;
     mask   : agar.core.types.uint32_t) return checkbox_access_t
  is
    ca_label : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate_flags32
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (ca_label'unchecked_access),
       ptr    => ptr,
       mask   => mask);
  end allocate_flags32;

  function widget (checkbox : checkbox_access_t) return widget_access_t is
  begin
    return checkbox.widget'access;
  end widget;

end agar.gui.widget.checkbox;
