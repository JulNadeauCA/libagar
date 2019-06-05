with agar.core.slist;
with agar.core.types;
with agar.gui.text;

package agar.gui.widget.label is

  use type c.unsigned;

  type flag_t is limited private;
  type flag_access_t is access all flag_t;
  pragma convention (c, flag_access_t);

  type label_t is limited private;
  type label_access_t is access all label_t;
  pragma convention (c, label_access_t);

  type format_spec_t is limited private;
  type format_spec_access_t is access all format_spec_t;
  pragma convention (c, format_spec_access_t);

  package flag_slist is new agar.core.slist
    (entry_type => flag_access_t);

  subtype mask_t is c.unsigned;

  type type_t is (
    LABEL_STATIC,
    LABEL_POLLED,
    LABEL_POLLED_MT
  );
  for type_t use (
    LABEL_STATIC    => 0,
    LABEL_POLLED    => 1,
    LABEL_POLLED_MT => 2
  );
  for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  type format_func_t is access procedure
    (label : label_access_t;
     str   : cs.chars_ptr;
     size  : c.size_t;
     num   : c.int);
  pragma convention (c, format_func_t);

  type flags_t is new c.unsigned;
  LABEL_HFILL     : constant flags_t := 16#01#;
  LABEL_VFILL     : constant flags_t := 16#02#;
  LABEL_NOMINSIZE : constant flags_t := 16#04#;
  LABEL_PARTIAL   : constant flags_t := 16#10#;
  LABEL_REGEN     : constant flags_t := 16#20#;
  LABEL_FRAME     : constant flags_t := 16#80#;
  LABEL_EXPAND    : constant flags_t := LABEL_HFILL or LABEL_VFILL;

  max          : constant := 1024;
  max_pollptrs : constant := 32;

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t;
     text   : string) return label_access_t;
  pragma import (c, allocate, "AG_LabelNewS");

  function allocate_polled
    (parent : widget_access_t;
     flags  : flags_t;
     text   : string) return label_access_t;
  pragma inline (allocate_polled);

  function allocate_polled_mutex
    (parent : widget_access_t;
     flags  : flags_t;
     mutex  : agar.core.threads.mutex_t;
     text   : string) return label_access_t;
  pragma inline (allocate_polled_mutex);

  procedure set_padding
    (label  : label_access_t;
     left   : natural;
     right  : natural;
     top    : natural;
     bottom : natural);
  pragma inline (set_padding);

  procedure justify
    (label   : label_access_t;
     justify : agar.gui.text.justify_t);
  pragma import (c, justify, "AG_LabelJustify");

  procedure size_hint
    (label     : label_access_t;
     num_lines : natural;
     text      : string);
  pragma inline (size_hint);

  -- static labels

  procedure text
    (label : label_access_t;
     text  : string);
  pragma inline (text);

  -- flag descriptions

  procedure flag
    (label : label_access_t;
     index : natural;
     desc  : string;
     mask  : mask_t);
  pragma inline (flag);

  procedure flag8
    (label : label_access_t;
     index : natural;
     desc  : string;
     mask  : agar.core.types.uint8_t);
  pragma inline (flag8);

  procedure flag16
    (label : label_access_t;
     index : natural;
     desc  : string;
     mask  : agar.core.types.uint16_t);
  pragma inline (flag16);

  procedure flag32
    (label : label_access_t;
     index : natural;
     desc  : string;
     mask  : agar.core.types.uint32_t);
  pragma inline (flag32);

  function widget (label : label_access_t) return widget_access_t;
  pragma inline (widget);

private

  type flag_t is record
    index     : c.unsigned;
    text      : cs.chars_ptr;
    v         : agar.core.types.uint32_t;
    bind_type : binding_type_t;
    lflags    : flag_slist.entry_t;
  end record;
  pragma convention (c, flag_t);

  type format_spec_t is record
    fmt        : cs.chars_ptr;
    fmt_length : c.size_t;
    func       : format_func_t;
  end record;
  pragma convention (c, format_spec_t);

  type poll_ptrs_t is array (1 .. max_pollptrs) of aliased agar.core.types.void_ptr_t;
  pragma convention (c, poll_ptrs_t);

  type poll_t is record
    lock  : agar.core.threads.mutex_t;
    ptrs  : poll_ptrs_t;
    nptrs : c.int;
  end record;
  pragma convention (c, poll_t);

  type label_t is record
    widget       : aliased widget_t;
    label_type   : type_t;
    flags        : flags_t;
    text         : cs.chars_ptr;
    surface      : c.int;
    surface_cont : c.int;
    width_pre    : c.int;
    height_pre   : c.int;
    pad_left     : c.int;
    pad_right    : c.int;
    pad_top      : c.int;
    pad_bottom   : c.int;
    justify      : agar.gui.text.justify_t;
    valign       : agar.gui.text.valign_t;
    poll         : poll_t;
    lflags       : flag_slist.head_t;
    cache        : agar.core.types.void_ptr_t; -- XXX: ag_text_cache *
    r_clip       : agar.gui.rect.rect_t;
  end record;
  pragma convention (c, label_t);

end agar.gui.widget.label;
