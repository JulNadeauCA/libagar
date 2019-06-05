#include <agar/core.h>
#include <agar/gui.h>

AG_MenuItem *
agar_gui_widget_menu_bool (AG_MenuItem *item, const char *text, AG_Surface *icon,
  int *value, int invert)
{
  return AG_MenuBool (item, text, icon, value, invert);
}

AG_MenuItem *
agar_gui_widget_menu_int_flags (AG_MenuItem *item, const char *text,
  AG_Surface *icon, int *value, int flags, int invert)
{
  return AG_MenuIntFlags (item, text, icon, value, flags, invert);
}

AG_MenuItem *
agar_gui_widget_menu_int_flags8 (AG_MenuItem *item, const char *text,
  AG_Surface *icon, Uint8 *value, Uint8 flags, int invert)
{
  return AG_MenuInt8Flags (item, text, icon, value, flags, invert);
}

AG_MenuItem *
agar_gui_widget_menu_int_flags16 (AG_MenuItem *item, const char *text,
  AG_Surface *icon, Uint16 *value, Uint16 flags, int invert)
{
  return AG_MenuInt16Flags (item, text, icon, value, flags, invert);
}

AG_MenuItem *
agar_gui_widget_menu_int_flags32 (AG_MenuItem *item, const char *text,
  AG_Surface *icon, Uint32 *value, Uint32 flags, int invert)
{
  return AG_MenuInt32Flags (item, text, icon, value, flags, invert);
}
