package body agar.gui.widget.table is

  use type c.int;

  package cbinds is
    function allocate_polled
      (parent   : widget_access_t;
       flags    : table_flags_t;
       callback : agar.core.event.callback_t;
       fmt      : agar.core.types.void_ptr_t) return table_access_t;
    pragma import (c, allocate_polled, "AG_TableNewPolled"); 

    procedure size_hint
      (table    : table_access_t;
       width    : c.int;
       num_rows : c.int);
    pragma import (c, size_hint, "AG_TableSizeHint");

    procedure set_separator
      (table     : table_access_t;
       separator : cs.chars_ptr);
    pragma import (c, set_separator, "AG_TableSetSeparator"); 

    function set_popup
      (table  : table_access_t;
       row    : c.int;
       column : c.int) return agar.gui.widget.menu.item_access_t;
    pragma import (c, set_popup, "AG_TableSetPopup"); 

    procedure set_row_double_click_func
      (table    : table_access_t;
       callback : agar.core.event.callback_t;
       fmt      : agar.core.types.void_ptr_t);
    pragma import (c, set_row_double_click_func, "AG_TableSetRowDblClickFn");

    procedure set_column_double_click_func
      (table    : table_access_t;
       callback : agar.core.event.callback_t;
       fmt      : agar.core.types.void_ptr_t);
    pragma import (c, set_column_double_click_func, "AG_TableSetColDblClickFn");

    function add_column
      (table         : table_access_t;
       name          : cs.chars_ptr;
       size_spec     : cs.chars_ptr;
       sort_callback : sort_callback_t) return c.int;
    pragma import (c, add_column, "AG_TableAddCol"); 

    procedure select_column
      (table  : table_access_t;
       column : c.int);
    pragma import (c, select_column, "agar_gui_widget_table_select_col"); 

    procedure deselect_column
      (table  : table_access_t;
       column : c.int);
    pragma import (c, deselect_column, "agar_gui_widget_table_deselect_col");

    function column_selected
      (table  : table_access_t;
       column : c.int) return c.int;
    pragma import (c, column_selected, "agar_gui_widget_table_col_selected"); 

    procedure select_row
      (table : table_access_t;
       row   : c.unsigned);
    pragma import (c, select_row, "AG_TableSelectRow");

    procedure deselect_row
      (table : table_access_t;
       row   : c.unsigned);
    pragma import (c, deselect_row, "AG_TableDeselectRow");

    function row_selected
      (table : table_access_t;
       row   : c.unsigned) return c.int;
    pragma import (c, row_selected, "AG_TableRowSelected");

    procedure select_cell
      (table  : table_access_t;
       row    : c.unsigned;
       column : c.unsigned);
    pragma import (c, select_cell, "agar_gui_widget_table_select_cell");

    procedure deselect_cell
      (table  : table_access_t;
       row    : c.unsigned;
       column : c.unsigned);
    pragma import (c, deselect_cell, "agar_gui_widget_table_deselect_cell");

    function cell_selected
      (table  : table_access_t;
       row    : c.unsigned;
       column : c.unsigned) return c.int;
    pragma import (c, cell_selected, "agar_gui_widget_table_cell_selected");

    function compare_cells
      (cell1 : cell_access_t;
       cell2 : cell_access_t) return c.int;
    pragma import (c, compare_cells, "AG_TableCompareCells");
  end cbinds;

  function allocate_polled  
    (parent   : widget_access_t;
     flags    : table_flags_t;
     callback : agar.core.event.callback_t) return table_access_t is
  begin
    return cbinds.allocate_polled
      (parent   => parent,
       flags    => flags,
       callback => callback,
       fmt      => agar.core.types.null_ptr);
  end allocate_polled;

  procedure size_hint
    (table    : table_access_t;
     width    : positive;
     num_rows : positive) is
  begin
    cbinds.size_hint
      (table    => table,
       width    => c.int (width),
       num_rows => c.int (num_rows));
  end size_hint;

  procedure set_separator
    (table     : table_access_t;
     separator : string)
  is
    c_sep : aliased c.char_array := c.to_c (separator);
  begin
    cbinds.set_separator
      (table     => table,
       separator => cs.to_chars_ptr (c_sep'unchecked_access));
  end set_separator;

  function set_popup
    (table  : table_access_t;
     row    : integer;
     column : integer) return agar.gui.widget.menu.item_access_t is
  begin
    return cbinds.set_popup
      (table  => table,
       row    => c.int (row),
       column => c.int (column));
  end set_popup;

  procedure set_row_double_click_func
    (table    : table_access_t;
     callback : agar.core.event.callback_t) is
  begin
    cbinds.set_row_double_click_func
      (table    => table,
       callback => callback,
       fmt      => agar.core.types.null_ptr);
  end set_row_double_click_func;

  procedure set_column_double_click_func
    (table    : table_access_t;
     callback : agar.core.event.callback_t) is
  begin
    cbinds.set_column_double_click_func
      (table    => table,
       callback => callback,
       fmt      => agar.core.types.null_ptr);
  end set_column_double_click_func;

  -- column functions

  function add_column
    (table         : table_access_t;
     name          : string;
     size_spec     : string;
     sort_callback : sort_callback_t) return boolean
  is
    c_name : aliased c.char_array := c.to_c (name);
    c_spec : aliased c.char_array := c.to_c (size_spec);
  begin
    return cbinds.add_column
      (table         => table,
       name          => cs.to_chars_ptr (c_name'unchecked_access),
       size_spec     => cs.to_chars_ptr (c_spec'unchecked_access),
       sort_callback => sort_callback) = 0;
  end add_column;

  procedure select_column
    (table  : table_access_t;
     column : integer) is
  begin
    cbinds.select_column
      (table  => table,
       column => c.int (column));
  end select_column;

  procedure deselect_column
    (table  : table_access_t;
     column : integer) is
  begin
    cbinds.deselect_column
      (table  => table,
       column => c.int (column));
  end deselect_column;

  function column_selected
    (table  : table_access_t;
     column : integer) return boolean is
  begin
    return cbinds.column_selected
      (table  => table,
       column => c.int (column)) = 0;
  end column_selected;

  -- row functions

  procedure select_row
    (table : table_access_t;
     row   : natural) is
  begin
    cbinds.select_row
      (table => table,
       row   => c.unsigned (row));
  end select_row;

  procedure deselect_row
    (table : table_access_t;
     row   : natural) is
  begin
    cbinds.deselect_row
      (table => table,
       row   => c.unsigned (row));
  end deselect_row;

  function row_selected
    (table : table_access_t;
     row   : natural) return boolean is
  begin
    return cbinds.row_selected
      (table => table,
       row   => c.unsigned (row)) = 1;
  end row_selected;

  -- cell functions

  procedure select_cell
    (table  : table_access_t;
     row    : natural;
     column : natural) is
  begin
    cbinds.select_cell
      (table  => table,
       row    => c.unsigned (row),
       column => c.unsigned (column));
  end select_cell;

  procedure deselect_cell
    (table  : table_access_t;
     row    : natural;
     column : natural) is
  begin
    cbinds.deselect_cell
      (table  => table,
       row    => c.unsigned (row),
       column => c.unsigned (column));
  end deselect_cell;

  function cell_selected
    (table  : table_access_t;
     row    : natural;
     column : natural) return boolean is
  begin
    return cbinds.cell_selected
      (table  => table,
       row    => c.unsigned (row),
       column => c.unsigned (column)) = 1;
  end cell_selected;

  function compare_cells
    (cell1 : cell_access_t;
     cell2 : cell_access_t) return integer is
  begin
    return integer (cbinds.compare_cells
      (cell1 => cell1,
       cell2 => cell2));
  end compare_cells;

  function rows (table : table_access_t) return natural is
  begin
    return natural (table.m);
  end rows;

  function columns (table : table_access_t) return natural is
  begin
    return natural (table.n);
  end columns;

  function widget (table : table_access_t) return widget_access_t is
  begin
    return table.widget'access;
  end widget;

end agar.gui.widget.table;
