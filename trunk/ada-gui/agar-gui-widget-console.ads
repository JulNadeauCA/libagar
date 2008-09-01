with agar.core.types;
with agar.gui.text;
with agar.gui.widget.scrollbar;

package agar.gui.widget.console is

  use type c.unsigned;

  type console_t;
  type console_access_t is access all console_t;
  pragma convention (c, console_access_t);

  type line_t is record
    text     : cs.chars_ptr;
    len      : c.size_t;
    surface  : c.int;
    selected : c.int;
    icon     : c.int;
    font     : agar.gui.text.font_access_t;
    color_fg : agar.core.types.uint32_t;
    color_bg : agar.core.types.uint32_t;
    ptr      : agar.core.types.void_ptr_t;
    cons     : console_access_t;
  end record;
  type line_access_t is access all line_t;
  pragma convention (c, line_t);
  pragma convention (c, line_access_t);

  subtype flags_t is c.unsigned;
  CONSOLE_HFILL  : constant flags_t := 16#01#;
  CONSOLE_VFILL  : constant flags_t := 16#02#;
  CONSOLE_EXPAND : constant flags_t := CONSOLE_HFILL or CONSOLE_VFILL;

  type console_t is record
    widget       : widget_t;
    flags        : flags_t;
    padding      : c.int;
    lineskip     : c.int;
    lines        : line_access_t;
    lines_number : c.unsigned;
    row_offset   : c.unsigned;
    color_bg     : agar.core.types.uint32_t;
    scrollbar    : agar.gui.widget.scrollbar.scrollbar_access_t;
  end record;
  pragma convention (c, console_t);

end agar.gui.widget.console;
