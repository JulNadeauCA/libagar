#include <agar/core.h>
#include <agar/gui.h>

void
agar_gui_text_font (AG_Font *font)
{
  AG_TextFont (font);
}

AG_Surface *
agar_gui_text_render (const char *text)
{
  return AG_TextRender (text);
}
