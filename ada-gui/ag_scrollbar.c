#include <agar/core.h>
#include <agar/gui.h>

void
agar_gui_widget_scrollbar_set_size (AG_Scrollbar *sb, int bsize)
{
  AG_ScrollbarSetBarSize (sb, bsize);
}

int
agar_gui_widget_scrollbar_get_size (AG_Scrollbar *sb)
{
  return AG_ScrollbarGetBarSize (sb);
}
