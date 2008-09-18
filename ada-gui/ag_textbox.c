#include <agar/core.h>
#include <agar/gui.h>

void
agar_gui_widget_textbox_set_static (AG_Textbox *textbox, int enable)
{
  AG_TextboxSetStatic (textbox, enable);
}

void
agar_gui_widget_textbox_set_password (AG_Textbox *textbox, int enable)
{
  AG_TextboxSetPassword (textbox, enable);
}

void
agar_gui_widget_textbox_set_float_only (AG_Textbox *textbox, int enable)
{
  AG_TextboxSetFltOnly (textbox, enable);
}

void
agar_gui_widget_textbox_set_integer_only (AG_Textbox *textbox, int enable)
{
  AG_TextboxSetIntOnly (textbox, enable);
}

void
agar_gui_widget_textbox_size_hint (AG_Textbox *textbox, const char *text)
{
  AG_TextboxSizeHint (textbox, text);
}

void
agar_gui_widget_textbox_size_hint_pixels (AG_Textbox *textbox,
  unsigned int w, unsigned int h)
{
  AG_TextboxSizeHintPixels (textbox, w, h);
}

int
agar_gui_widget_textbox_map_position (AG_Textbox *textbox, int x, int y,
  int *pos, int absolute)
{
  return AG_TextboxMapPosition (textbox, x, y, pos, absolute);
}

void
agar_gui_widget_textbox_move_cursor (AG_Textbox *textbox, int x, int y,
  int absolute)
{
  AG_TextboxMoveCursor (textbox, x, y, absolute);
}

int
agar_gui_widget_textbox_get_cursor_pos (AG_Textbox *textbox)
{
  return AG_TextboxGetCursorPos (textbox);
}

int
agar_gui_widget_textbox_set_cursor_pos (AG_Textbox *textbox, int pos)
{
  return AG_TextboxSetCursorPos (textbox, pos);
}

int
agar_gui_widget_textbox_int (AG_Textbox *textbox)
{
  return AG_TextboxInt (textbox);
}

float
agar_gui_widget_textbox_float (AG_Textbox *textbox)
{
  return AG_TextboxFlt (textbox);
}

double
agar_gui_widget_textbox_double (AG_Textbox *textbox)
{
  return AG_TextboxDbl (textbox);
}

void
agar_gui_widget_textbox_set_string (AG_Textbox *textbox, const char *str)
{
  AG_TextboxSetString (textbox, str);
}

void
agar_gui_widget_textbox_set_string_ucs4 (AG_Textbox *textbox, Uint32 *str)
{
  AG_TextboxSetStringUCS4 (textbox, str);
}
