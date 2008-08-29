#include <agar/core.h>
#include <agar/gui.h>

AG_Button *
agar_gui_widget_new_int (AG_Widget *parent, unsigned int flags, const char *label, int *p)
{
  return AG_ButtonNewInt (parent, flags, label, p);
}

AG_Button *
agar_gui_widget_new_uint8 (AG_Widget *parent, unsigned int flags, const char *label, Uint8 *p)
{
  return AG_ButtonNewUint8 (parent, flags, label, p);
}

AG_Button *
agar_gui_widget_new_uint16 (AG_Widget *parent, unsigned int flags, const char *label, Uint16 *p)
{
  return AG_ButtonNewUint16 (parent, flags, label, p);
}

AG_Button *
agar_gui_widget_new_uint32 (AG_Widget *parent, unsigned int flags, const char *label, Uint32 *p)
{
  return AG_ButtonNewUint32 (parent, flags, label, p);
}
