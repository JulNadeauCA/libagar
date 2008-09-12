#include <agar/core.h>
#include <agar/gui.h>

AG_HBox *
agar_gui_widget_hbox_new (AG_Widget *parent, int flags)
{
  return AG_HBoxNew (parent, flags);
}

void
agar_gui_widget_hbox_set_homogenous (AG_HBox *box, int homogenous)
{
  AG_HBoxSetHomogenous (box, homogenous);
}

void
agar_gui_widget_hbox_set_padding (AG_HBox *box, int padding)
{
  AG_HBoxSetPadding (box, padding);
}

void
agar_gui_widget_hbox_set_spacing (AG_HBox *box, int spacing)
{
  AG_HBoxSetSpacing (box, spacing);
}
