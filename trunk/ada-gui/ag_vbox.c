#include <agar/core.h>
#include <agar/gui.h>

AG_VBox *
agar_gui_widget_vbox_new (AG_Widget *parent, int flags)
{
  return AG_VBoxNew (parent, flags);
}

void
agar_gui_widget_vbox_set_homogenous (AG_VBox *box, int homogenous)
{
  AG_VBoxSetHomogenous (box, homogenous);
}

void
agar_gui_widget_vbox_set_padding (AG_VBox *box, int padding)
{
  AG_VBoxSetPadding (box, padding);
}

void
agar_gui_widget_vbox_set_spacing (AG_VBox *box, int spacing)
{
  AG_VBoxSetSpacing (box, spacing);
}
