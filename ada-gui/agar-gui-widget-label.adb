package body agar.gui.widget.label is

  package cbinds is
    function allocate_polled
      (parent : widget_access_t;
       flags  : flags_t;
       fmt    : cs.chars_ptr;
       text   : cs.chars_ptr) return label_access_t;
    pragma import (c, allocate_polled, "AG_LabelNewPolled");

    function allocate_polled_mutex
      (parent : widget_access_t;
       flags  : flags_t;
       mutex  : agar.core.threads.mutex_t;
       fmt    : cs.chars_ptr;
       text   : cs.chars_ptr) return label_access_t;
    pragma import (c, allocate_polled_mutex, "AG_LabelNewPolledMT");

    procedure set_padding
      (label  : label_access_t;
       left   : c.int;
       right  : c.int;
       top    : c.int;
       bottom : c.int);
    pragma import (c, set_padding, "AG_LabelSetPadding");

    procedure size_hint
      (label     : label_access_t;
       num_lines : c.unsigned;
       text      : cs.chars_ptr);
    pragma import (c, size_hint, "AG_LabelSizeHint");

    procedure text
      (label : label_access_t;
       text  : cs.chars_ptr);
    pragma import (c, text, "AG_LabelTextS");

    procedure flag
      (label : label_access_t;
       index : c.unsigned;
       desc  : cs.chars_ptr;
       mask  : mask_t);
    pragma import (c, flag, "agar_gui_widget_label_flag");

    procedure flag8
      (label : label_access_t;
       index : c.unsigned;
       desc  : cs.chars_ptr;
       mask  : agar.core.types.uint8_t);
    pragma import (c, flag8, "agar_gui_widget_label_flag8");

    procedure flag16
      (label : label_access_t;
       index : c.unsigned;
       desc  : cs.chars_ptr;
       mask  : agar.core.types.uint16_t);
    pragma import (c, flag16, "agar_gui_widget_label_flag16");

    procedure flag32
      (label : label_access_t;
       index : c.unsigned;
       desc  : cs.chars_ptr;
       mask  : agar.core.types.uint32_t);
    pragma import (c, flag32, "agar_gui_widget_label_flag32");
  end cbinds;

  function allocate_polled
    (parent : widget_access_t;
     flags  : flags_t;
     text   : string) return label_access_t
  is
    ca_text : aliased c.char_array := c.to_c (text);
    ca_fmt  : aliased c.char_array := c.to_c ("%s");
  begin
    return cbinds.allocate_polled
      (parent => parent,
       flags  => flags,
       fmt    => cs.to_chars_ptr (ca_fmt'unchecked_access),
       text   => cs.to_chars_ptr (ca_text'unchecked_access));
  end allocate_polled;

  function allocate_polled_mutex
    (parent : widget_access_t;
     flags  : flags_t;
     mutex  : agar.core.threads.mutex_t;
     text   : string) return label_access_t
  is
    ca_text : aliased c.char_array := c.to_c (text);
    ca_fmt  : aliased c.char_array := c.to_c ("%s");
  begin
    return cbinds.allocate_polled_mutex
      (parent => parent,
       flags  => flags,
       mutex  => mutex,
       fmt    => cs.to_chars_ptr (ca_fmt'unchecked_access),
       text   => cs.to_chars_ptr (ca_text'unchecked_access));
  end allocate_polled_mutex;

  procedure set_padding
    (label  : label_access_t;
     left   : natural;
     right  : natural;
     top    : natural;
     bottom : natural) is
  begin
    cbinds.set_padding
      (label  => label,
       left   => c.int (left),
       right  => c.int (right),
       top    => c.int (top),
       bottom => c.int (bottom));
  end set_padding;

  procedure size_hint
    (label     : label_access_t;
     num_lines : natural;
     text      : string)
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.size_hint
      (label     => label,
       num_lines => c.unsigned (num_lines),
       text      => cs.to_chars_ptr (ca_text'unchecked_access));
  end size_hint;

  -- static labels

  procedure text
    (label : label_access_t;
     text  : string)
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.text (label, cs.to_chars_ptr (ca_text'unchecked_access));
  end text;

  -- flag descriptions

  procedure flag
    (label : label_access_t;
     index : natural;
     desc  : string;
     mask  : mask_t)
  is
    ca_desc : aliased c.char_array := c.to_c (desc);
  begin
    cbinds.flag
      (label => label,
       index => c.unsigned (index),
       desc  => cs.to_chars_ptr (ca_desc'unchecked_access),
       mask  => mask);
  end flag;

  procedure flag8
    (label : label_access_t;
     index : natural;
     desc  : string;
     mask  : agar.core.types.uint8_t)
  is
    ca_desc : aliased c.char_array := c.to_c (desc);
  begin
    cbinds.flag8
      (label => label,
       index => c.unsigned (index),
       desc  => cs.to_chars_ptr (ca_desc'unchecked_access),
       mask  => mask);
  end flag8;

  procedure flag16
    (label : label_access_t;
     index : natural;
     desc  : string;
     mask  : agar.core.types.uint16_t)
  is
    ca_desc : aliased c.char_array := c.to_c (desc);
  begin
    cbinds.flag16
      (label => label,
       index => c.unsigned (index),
       desc  => cs.to_chars_ptr (ca_desc'unchecked_access),
       mask  => mask);
  end flag16;

  procedure flag32
    (label : label_access_t;
     index : natural;
     desc  : string;
     mask  : agar.core.types.uint32_t)
  is
    ca_desc : aliased c.char_array := c.to_c (desc);
  begin
    cbinds.flag32
      (label => label,
       index => c.unsigned (index),
       desc  => cs.to_chars_ptr (ca_desc'unchecked_access),
       mask  => mask);
  end flag32;

  function widget (label : label_access_t) return widget_access_t is
  begin
    return label.widget'access;
  end widget;

end agar.gui.widget.label;
