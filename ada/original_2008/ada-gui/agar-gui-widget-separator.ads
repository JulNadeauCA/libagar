package agar.gui.widget.separator is

  type type_t is (SEPARATOR_HORIZ, SEPARATOR_VERT);
  for type_t use (SEPARATOR_HORIZ => 0, SEPARATOR_VERT => 1);
  for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  type separator_t is limited private;
  type separator_access_t is access all separator_t;
  pragma convention (c, separator_access_t);

  function allocate
    (parent         : widget_access_t;
     separator_type : type_t) return separator_access_t;
  pragma import (c, allocate, "AG_SeparatorNew");

  function allocate_spacer
    (parent         : widget_access_t;
     separator_type : type_t) return separator_access_t;
  pragma import (c, allocate_spacer, "AG_SpacerNew");

  procedure set_padding
    (separator : separator_access_t;
     pixels    : natural);
  pragma inline (set_padding);

  function widget (separator : separator_access_t) return widget_access_t;
  pragma inline (widget);

private

  type separator_t is record
    widget   : aliased widget_t;
    sep_type : type_t;
    padding  : c.unsigned;
    visible  : c.int;
  end record;
  pragma convention (c, separator_t);

end agar.gui.widget.separator;
