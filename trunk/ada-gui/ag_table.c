#include <agar/core.h>
#include <agar/gui.h>

int
agar_gui_widget_table_col_selected (AG_Table *tbl, int col)
{
  return AG_TableColSelected (tbl, col);
}

void
agar_gui_widget_table_deselect_col (AG_Table *tbl, int col)
{
  AG_TableDeselectCol (tbl, col);
}

void
agar_gui_widget_table_select_col (AG_Table *tbl, int col)
{
  AG_TableSelectCol (tbl, col);
}

int
agar_gui_widget_table_cell_selected (AG_Table *tbl,
  unsigned int row, unsigned int col)
{
  return AG_TableCellSelected (tbl, row, col);
}

void
agar_gui_widget_table_deselect_cell (AG_Table *tbl,
  unsigned int row, unsigned int col)
{
  AG_TableDeselectCell (tbl, row, col);
}

void
agar_gui_widget_table_select_cell (AG_Table *tbl,
  unsigned int row, unsigned int col)
{
  AG_TableSelectCell (tbl, row, col);
}
