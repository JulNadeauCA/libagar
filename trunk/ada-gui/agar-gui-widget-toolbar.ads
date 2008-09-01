with agar.gui.widget.box;

package agar.gui.widget.toolbar is

  type type_t is (TOOLBAR_HORIZ, TOOLBAR_VERT);
   for type_t use (TOOLBAR_HORIZ => 0, TOOLBAR_VERT => 1);
   for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  subtype flags_t is c.unsigned;
  TOOLBAR_HOMOGENOUS   : constant flags_t := 16#01#;
  TOOLBAR_STICKY       : constant flags_t := 16#02#;
  TOOLBAR_MULTI_STICKY : constant flags_t := 16#04#;

  type rows_t is array (1 .. 8) of aliased agar.gui.widget.box.box_access_t;
  pragma convention (c, rows_t);

  type toolbar_t is record
    box          : agar.gui.widget.box.box_t;
    rows         : rows_t;
    toolbar_type : type_t;
    num_rows     : c.int;
    num_buttons  : c.int;
    row_current  : c.int;
    flags        : flags_t;
  end record;
  type toolbar_access_t is access all toolbar_t;
  pragma convention (c, toolbar_t);
  pragma convention (c, toolbar_access_t);

end agar.gui.widget.toolbar;
