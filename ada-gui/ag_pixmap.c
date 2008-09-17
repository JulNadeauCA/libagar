#include <agar/core.h>
#include <agar/gui.h>

void
agar_gui_widget_pixmap_set_surface (AG_Pixmap *pm, int name)
{
  AG_PixmapSetSurface (pm, name);
}

void
agar_gui_widget_pixmap_replace_surface (AG_Pixmap *pm, int name, AG_Surface *surf)
{
  AG_PixmapReplaceSurface (pm, name, surf);
}

void
agar_gui_widget_pixmap_update_surface (AG_Pixmap *pm, int name)
{
  AG_PixmapUpdateSurface (pm, name);
}

void
agar_gui_widget_pixmap_set_coords (AG_Pixmap *pm, int s, int t)
{
  AG_PixmapSetCoords (pm, s, t);
}
