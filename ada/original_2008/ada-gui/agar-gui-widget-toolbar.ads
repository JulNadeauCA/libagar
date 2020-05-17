with agar.gui.widget.box;
with agar.gui.widget.button;

package agar.gui.widget.toolbar is

  type type_t is (TOOLBAR_HORIZ, TOOLBAR_VERT);
   for type_t use (TOOLBAR_HORIZ => 0, TOOLBAR_VERT => 1);
   for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  type flags_t is new c.unsigned;
  TOOLBAR_HOMOGENOUS   : constant flags_t := 16#01#;
  TOOLBAR_STICKY       : constant flags_t := 16#02#;
  TOOLBAR_MULTI_STICKY : constant flags_t := 16#04#;

  type toolbar_t is limited private;
  type toolbar_access_t is access all toolbar_t;
  pragma convention (c, toolbar_access_t);

  -- API

  function allocate
    (parent   : widget_access_t;
     bar_type : type_t;
     num_rows : natural;
     flags    : flags_t) return toolbar_access_t;
  pragma inline (allocate);

  procedure row
    (toolbar  : toolbar_access_t;
     row_name : natural);
  pragma inline (row);

  procedure separator (toolbar : toolbar_access_t);
  pragma import (c, separator, "AG_ToolbarSeparator");

  procedure bar_select
    (toolbar : toolbar_access_t;
     button  : agar.gui.widget.button.button_access_t);
  pragma import (c, bar_select, "AG_ToolbarSelect");

  procedure bar_deselect
    (toolbar : toolbar_access_t;
     button  : agar.gui.widget.button.button_access_t);
  pragma import (c, bar_deselect, "AG_ToolbarDeselect");

  procedure bar_select_only
    (toolbar : toolbar_access_t;
     button  : agar.gui.widget.button.button_access_t);
  pragma import (c, bar_select_only, "AG_ToolbarSelectOnly");

  procedure bar_select_all (toolbar : toolbar_access_t);
  pragma import (c, bar_select_all, "AG_ToolbarSelectAll");

  procedure bar_deselect_all (toolbar : toolbar_access_t);
  pragma import (c, bar_deselect_all, "AG_ToolbarDeselectAll");

  --

  function widget (toolbar : toolbar_access_t) return widget_access_t;
  pragma inline (widget);

private

  type rows_t is array (1 .. 8) of aliased agar.gui.widget.box.box_access_t;
  pragma convention (c, rows_t);

  type toolbar_t is record
    box          : aliased agar.gui.widget.box.box_t;
    rows         : rows_t;
    toolbar_type : type_t;
    num_rows     : c.int;
    num_buttons  : c.int;
    row_current  : c.int;
    flags        : flags_t;
  end record;
  pragma convention (c, toolbar_t);

end agar.gui.widget.toolbar;
