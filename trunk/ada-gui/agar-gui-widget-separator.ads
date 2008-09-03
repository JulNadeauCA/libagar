package agar.gui.widget.separator is

  type type_t is (SEPARATOR_HORIZ, SEPARATOR_VERT);
  for type_t use (SEPARATOR_HORIZ => 0, SEPARATOR_VERT => 1);
  for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  type separator_t is record
    widget   : widget_t;
    sep_type : type_t;
    padding  : c.unsigned;
    visible  : c.int;
  end record;
  type separator_access_t is access all separator_t;
  pragma convention (c, separator_t);

end agar.gui.widget.separator;
