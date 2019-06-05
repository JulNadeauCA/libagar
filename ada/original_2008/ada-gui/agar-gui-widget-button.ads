with agar.core.event;
with agar.core.timeout;
with agar.core.types;
with agar.gui.surface;
with agar.gui.text;

package agar.gui.widget.button is

  use type c.unsigned;

  type flags_t is new c.unsigned;
  BUTTON_STICKY        : constant flags_t := 16#002#;
  BUTTON_MOUSEOVER     : constant flags_t := 16#004#;
  BUTTON_REPEAT        : constant flags_t := 16#008#;
  BUTTON_HFILL         : constant flags_t := 16#010#;
  BUTTON_VFILL         : constant flags_t := 16#020#;
  BUTTON_INVSTATE      : constant flags_t := 16#400#;
  BUTTON_EXPAND        : constant flags_t := BUTTON_HFILL or BUTTON_VFILL;

  type button_t is limited private;
  type button_access_t is access all button_t;
  pragma convention (c, button_access_t);

  function allocate
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string) return button_access_t;
  pragma inline (allocate);

  function allocate_function
    (parent   : widget_access_t;
     flags    : flags_t := 0;
     label    : string;
     callback : agar.core.event.callback_t) return button_access_t;
  pragma inline (allocate_function);

  function allocate_integer
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access integer) return button_access_t;
  pragma inline (allocate_integer);

  function allocate_uint8
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access agar.core.types.uint8_t) return button_access_t;
  pragma inline (allocate_uint8);

  function allocate_uint16
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access agar.core.types.uint16_t) return button_access_t;
  pragma inline (allocate_uint16);

  function allocate_uint32
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access agar.core.types.uint32_t) return button_access_t;
  pragma inline (allocate_uint32);

  function allocate_flag
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access integer;
     mask   : integer) return button_access_t;
  pragma inline (allocate_flag);

  function allocate_flag8
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access agar.core.types.uint8_t;
     mask   : agar.core.types.uint8_t) return button_access_t;
  pragma inline (allocate_flag8);

  function allocate_flag16
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access agar.core.types.uint16_t;
     mask   : agar.core.types.uint16_t) return button_access_t;
  pragma inline (allocate_flag16);

  function allocate_flag32
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access agar.core.types.uint32_t;
     mask   : agar.core.types.uint32_t) return button_access_t;
  pragma inline (allocate_flag32);

  procedure set_padding
    (button     : button_access_t;
     left_pad   : natural;
     right_pad  : natural;
     top_pad    : natural;
     bottom_pad : natural);
  pragma inline (set_padding);

  procedure set_focusable
    (button : button_access_t;
     flag   : boolean);
  pragma inline (set_focusable);

  procedure set_sticky
    (button : button_access_t;
     flag   : boolean);
  pragma inline (set_sticky);

  procedure invert_state
    (button : button_access_t;
     flag   : boolean);
  pragma inline (invert_state);

  procedure justify
    (button  : button_access_t;
     justify : agar.gui.text.justify_t);
  pragma import (c, justify, "AG_ButtonJustify");

  procedure set_repeat_mode
    (button : button_access_t;
     flag   : boolean);
  pragma inline (set_repeat_mode);

  procedure surface
    (button  : button_access_t;
     surface : agar.gui.surface.surface_access_t);
  pragma import (c, surface, "AG_ButtonSurface");

  procedure surface_no_copy
    (button  : button_access_t;
     surface : agar.gui.surface.surface_access_t);
  pragma import (c, surface_no_copy, "AG_ButtonSurfaceNODUP");

  procedure text
    (button  : button_access_t;
     text    : string);
  pragma inline (text);

  function widget (button : button_access_t) return widget_access_t;
  pragma inline (widget);

private

  type button_t is record
    widget      : aliased widget_t;
    state       : c.int;
    text        : cs.chars_ptr;
    surface     : c.int;
    surface_src : agar.gui.surface.surface_access_t;
    justify     : agar.gui.text.justify_t;
    valign      : agar.gui.text.valign_t;
    flags       : flags_t;
    left_pad    : c.int;
    right_pad   : c.int;
    top_pad     : c.int;
    bottom_pad  : c.int;
    delay_to    : agar.core.timeout.timeout_t;
    repeat_to   : agar.core.timeout.timeout_t;
  end record;
  pragma convention (c, button_t);
 
end agar.gui.widget.button;
