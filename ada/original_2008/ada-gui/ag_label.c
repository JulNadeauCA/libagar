#include <agar/core.h>
#include <agar/gui.h>

void
agar_gui_widget_label_flag (AG_Label *label, unsigned int index,
  const char *desc, unsigned int bitmask)
{
  AG_LabelFlagNew (label, index, desc, AG_VARIABLE_P_FLAG, bitmask);
}

void
agar_gui_widget_label_flag8 (AG_Label *label, unsigned int index,
  const char *desc, Uint8 bitmask)
{
  AG_LabelFlag8 (label, index, desc, bitmask);
}

void
agar_gui_widget_label_flag16 (AG_Label *label, unsigned int index,
  const char *desc, Uint16 bitmask)
{
  AG_LabelFlag16 (label, index, desc, bitmask);
}

void
agar_gui_widget_label_flag32 (AG_Label *label, unsigned int index,
  const char *desc, Uint32 bitmask)
{
  AG_LabelFlag32 (label, index, desc, bitmask);
}
