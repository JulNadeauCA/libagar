#include <agar/core.h>
#include <agar/gui.h>

AG_Surface *
agar_gui_text_render (const char *text)
{
  return AG_TextRender (text);
}
