package body agar.gui.widget.button is

  package cbinds is
    function allocate
      (parent : widget_access_t;
       flags  : flags_t := 0;
       label  : cs.chars_ptr) return button_access_t;
    pragma import (c, allocate, "AG_ButtonNewS");

    function allocate_function
      (parent   : widget_access_t;
       flags    : flags_t := 0;
       label    : cs.chars_ptr;
       callback : agar.core.event.callback_t) return button_access_t;
    pragma import (c, allocate_function, "AG_ButtonNewFn");

    function allocate_integer
      (parent : widget_access_t;
       flags  : flags_t := 0;
       label  : cs.chars_ptr;
       ptr    : access c.int) return button_access_t;
    pragma import (c, allocate_integer, "agar_gui_widget_new_int");

    function allocate_uint8
      (parent : widget_access_t;
       flags  : flags_t := 0;
       label  : cs.chars_ptr;
       ptr    : access agar.core.types.uint8_t) return button_access_t;
    pragma import (c, allocate_uint8, "agar_gui_widget_new_uint8");

    function allocate_uint16
      (parent : widget_access_t;
       flags  : flags_t := 0;
       label  : cs.chars_ptr;
       ptr    : access agar.core.types.uint16_t) return button_access_t;
    pragma import (c, allocate_uint16, "agar_gui_widget_new_uint16");

    function allocate_uint32
      (parent : widget_access_t;
       flags  : flags_t := 0;
       label  : cs.chars_ptr;
       ptr    : access agar.core.types.uint32_t) return button_access_t;
    pragma import (c, allocate_uint32, "agar_gui_widget_new_uint32");

    function allocate_flag
      (parent : widget_access_t;
       flags  : flags_t := 0;
       label  : cs.chars_ptr;
       ptr    : access c.int;
       mask   : c.unsigned) return button_access_t;
    pragma import (c, allocate_flag, "AG_ButtonNewFlag");

    function allocate_flag8
      (parent : widget_access_t;
       flags  : flags_t := 0;
       label  : cs.chars_ptr;
       ptr    : access agar.core.types.uint8_t;
       mask   : agar.core.types.uint8_t) return button_access_t;
    pragma import (c, allocate_flag8, "AG_ButtonNewFlag8");

    function allocate_flag16
      (parent : widget_access_t;
       flags  : flags_t := 0;
       label  : cs.chars_ptr;
       ptr    : access agar.core.types.uint16_t;
       mask   : agar.core.types.uint16_t) return button_access_t;
    pragma import (c, allocate_flag16, "AG_ButtonNewFlag16");

    function allocate_flag32
      (parent : widget_access_t;
       flags  : flags_t := 0;
       label  : cs.chars_ptr;
       ptr    : access agar.core.types.uint32_t;
       mask   : agar.core.types.uint32_t) return button_access_t;
    pragma import (c, allocate_flag32, "AG_ButtonNewFlag32");

    procedure set_padding
      (button     : button_access_t;
       left_pad   : c.int;
       right_pad  : c.int;
       top_pad    : c.int;
       bottom_pad : c.int);
    pragma import (c, set_padding, "AG_ButtonSetPadding");

    procedure set_focusable
      (button : button_access_t;
       flag   : c.int);
    pragma import (c, set_focusable, "AG_ButtonSetFocusable");

    procedure set_sticky
      (button : button_access_t;
       flag   : c.int);
    pragma import (c, set_sticky, "AG_ButtonSetSticky");

    procedure set_repeat_mode
      (button : button_access_t;
       flag   : c.int);
    pragma import (c, set_repeat_mode, "AG_ButtonSetRepeatMode");

    procedure invert_state
      (button : button_access_t;
       flag   : c.int);
    pragma import (c, invert_state, "AG_ButtonInvertState");

    procedure text
      (button : button_access_t;
       text   : cs.chars_ptr);
    pragma import (c, text, "AG_ButtonTextS");
  end cbinds;

  --

  function allocate
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string) return button_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (c_label'unchecked_access));
  end allocate;

  function allocate_function
    (parent   : widget_access_t;
     flags    : flags_t := 0;
     label    : string;
     callback : agar.core.event.callback_t) return button_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate_function
      (parent   => parent,
       callback => callback,
       flags    => flags,
       label    => cs.to_chars_ptr (c_label'unchecked_access));
  end allocate_function;

  function allocate_integer
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access integer) return button_access_t
  is
    c_label  : aliased c.char_array := c.to_c (label);
    c_int    : aliased c.int;
    c_button : button_access_t;
  begin
    c_button := cbinds.allocate_integer
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (c_label'unchecked_access),
       ptr    => c_int'unchecked_access);
    if c_button /= null then ptr.all := integer (c_int); end if;
    return c_button;
  end allocate_integer;

  function allocate_uint8
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access agar.core.types.uint8_t) return button_access_t
  is
    c_label   : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate_uint8
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (c_label'unchecked_access),
       ptr    => ptr);
  end allocate_uint8;

  function allocate_uint16
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access agar.core.types.uint16_t) return button_access_t
  is
    c_label   : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate_uint16
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (c_label'unchecked_access),
       ptr    => ptr);
  end allocate_uint16;

  function allocate_uint32
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access agar.core.types.uint32_t) return button_access_t
  is
    c_label   : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate_uint32
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (c_label'unchecked_access),
       ptr    => ptr);
  end allocate_uint32;

  function allocate_flag
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access integer;
     mask   : integer) return button_access_t
  is
    c_label  : aliased c.char_array := c.to_c (label);
    c_int    : aliased c.int;
    c_button : aliased button_access_t;
  begin
    c_button := cbinds.allocate_flag
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (c_label'unchecked_access),
       ptr    => c_int'unchecked_access,
       mask   => c.unsigned (mask));
    if c_button /= null then ptr.all := integer (c_int); end if;
    return c_button;
  end allocate_flag;

  function allocate_flag8
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access agar.core.types.uint8_t;
     mask   : agar.core.types.uint8_t) return button_access_t
  is
    c_label : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate_flag8
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (c_label'unchecked_access),
       ptr    => ptr,
       mask   => mask);
  end allocate_flag8;

  function allocate_flag16
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access agar.core.types.uint16_t;
     mask   : agar.core.types.uint16_t) return button_access_t
  is
    c_label   : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate_flag16
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (c_label'unchecked_access),
       ptr    => ptr,
       mask   => mask);
  end allocate_flag16;

  function allocate_flag32
    (parent : widget_access_t;
     flags  : flags_t := 0;
     label  : string;
     ptr    : access agar.core.types.uint32_t;
     mask   : agar.core.types.uint32_t) return button_access_t
  is
    c_label   : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate_flag32
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (c_label'unchecked_access),
       ptr    => ptr,
       mask   => mask);
  end allocate_flag32;

  procedure set_padding
    (button     : button_access_t;
     left_pad   : natural;
     right_pad  : natural;
     top_pad    : natural;
     bottom_pad : natural) is
  begin
    cbinds.set_padding
      (button     => button,
       left_pad   => c.int (left_pad),
       right_pad  => c.int (right_pad),
       top_pad    => c.int (top_pad),
       bottom_pad => c.int (bottom_pad));
  end set_padding;

  procedure set_focusable
    (button : button_access_t;
     flag   : boolean) is
  begin
    if flag then
      cbinds.set_focusable (button, 1);
    else
      cbinds.set_focusable (button, 0);
    end if;
  end set_focusable;

  procedure set_sticky
    (button : button_access_t;
     flag   : boolean) is
  begin
    if flag then
      cbinds.set_sticky (button, 1);
    else
      cbinds.set_sticky (button, 0);
    end if;
  end set_sticky;

  procedure invert_state
    (button : button_access_t;
     flag   : boolean) is
  begin
    if flag then
      cbinds.invert_state (button, 1);
    else
      cbinds.invert_state (button, 0);
    end if;
  end invert_state;

  procedure set_repeat_mode
    (button : button_access_t;
     flag   : boolean) is
  begin
    if flag then
      cbinds.set_repeat_mode (button, 1);
    else
      cbinds.set_repeat_mode (button, 0);
    end if;
  end set_repeat_mode;

  procedure text
    (button  : button_access_t;
     text    : string)
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.text
      (button => button,
       text   => cs.to_chars_ptr (ca_text'unchecked_access));
  end text;

  function widget (button : button_access_t) return widget_access_t is
  begin
    return button.widget'access;
  end widget;

end agar.gui.widget.button;
