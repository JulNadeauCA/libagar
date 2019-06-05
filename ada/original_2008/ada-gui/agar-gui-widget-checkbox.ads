with agar.core.event;

package agar.gui.widget.checkbox is

  use type c.unsigned;

  type flags_t is new c.unsigned;
  CHECKBOX_HFILL  : constant flags_t := 16#01#;
  CHECKBOX_VFILL  : constant flags_t := 16#02#;
  CHECKBOX_EXPAND : constant flags_t := CHECKBOX_HFILL or CHECKBOX_VFILL;
  CHECKBOX_SET    : constant flags_t := 16#04#;

  type checkbox_t is limited private;
  type checkbox_access_t is access all checkbox_t;
  pragma convention (c, checkbox_access_t);

  function allocate
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string) return checkbox_access_t;
  pragma inline (allocate);

  function allocate_function
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string;
     func   : agar.core.event.callback_t) return checkbox_access_t;
  pragma inline (allocate_function);

  function allocate_integer
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string;
     ptr    : access c.int) return checkbox_access_t;
  pragma inline (allocate_integer);

  function allocate_flags
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string;
     ptr    : access c.unsigned;
     mask   : c.unsigned) return checkbox_access_t;
  pragma inline (allocate_flags);

  function allocate_flags32
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string;
     ptr    : access agar.core.types.uint32_t;
     mask   : agar.core.types.uint32_t) return checkbox_access_t;
  pragma inline (allocate_flags32);

  procedure set_from_flags
    (parent     : widget_access_t;
     flags      : flags_t;
     ptr        : access c.unsigned;
     flags_desc : flag_descr_access_t);
  pragma import (c, set_from_flags, "AG_CheckboxSetFromFlags");

  procedure set_from_flags32
    (parent     : widget_access_t;
     flags      : flags_t;
     ptr        : access agar.core.types.uint32_t;
     flags_desc : flag_descr_access_t);
  pragma import (c, set_from_flags32, "AG_CheckboxSetFromFlags");

  procedure toggle (checkbox : checkbox_access_t);
  pragma import (c, toggle, "AG_CheckboxToggle");

  function widget (checkbox : checkbox_access_t) return widget_access_t;
  pragma inline (widget);

private

  type checkbox_t is record
    widget     : aliased widget_t;
    flags      : flags_t;
    state      : c.int;
    label_text : cs.chars_ptr;
    label      : c.int;
    spacing    : c.int;
  end record;
  pragma convention (c, checkbox_t);

end agar.gui.widget.checkbox;
