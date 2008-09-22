#include <agar/core.h>
#include <agar/gui.h>

void
agar_window_set_icon (AG_Window *win, AG_Surface *surface)
{
  AG_WindowSetIcon (win, surface);
}

void
agar_window_set_icon_no_copy (AG_Window *win, AG_Surface *surface)
{
  AG_WindowSetIconNODUP (win, surface);
}

void
agar_window_set_geometry_bounded (AG_Window *win, int x, int y, int w, int h)
{
  AG_WindowSetGeometryBounded (win, x, y, w, h);
}

void
agar_window_set_geometry_aligned_percent (AG_Window *win,
  enum ag_window_alignment alignment, int wPct, int hPct)
{
  AG_WindowSetGeometryAlignedPct (win, alignment, wPct, hPct);
}

void
agar_window_set_geometry_aligned (AG_Window *win,
  enum ag_window_alignment alignment, int w, int h)
{
  AG_WindowSetGeometryAligned (win, alignment, w, h);
}

void
agar_window_set_geometry (AG_Window *win, int x, int y, int w, int h)
{
  AG_WindowSetGeometry (win, x, y, w, h);
}

int
agar_window_is_focused (AG_Window *win)
{
  return AG_WindowIsFocused (win);
}

int
agar_window_is_visible (AG_Window *win)
{
  return AG_WindowIsVisible (win);
}
