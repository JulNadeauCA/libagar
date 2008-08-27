#include <agar/core.h>
#include <agar/gui.h>

void
agar_draw_box (AG_Widget *widget, AG_Rect r, int z, Uint32 c)
{
  AG_DrawBox (widget, r, z, c);
}

void
agar_draw_box_rounded (AG_Widget *widget, AG_Rect r, int z, int radius, Uint32 c)
{
  AG_DrawBoxRounded (widget, r, z, radius, c);
}

void
agar_draw_box_rounded_top (AG_Widget *widget, AG_Rect r, int z, int radius, Uint32 c)
{
  AG_DrawBoxRoundedTop (widget, r, z, radius, c);
}

void
agar_draw_frame (AG_Widget *widget, AG_Rect r, int z, Uint32 c)
{
  AG_DrawFrame (widget, r, z, c);
}

void
agar_draw_circle (AG_Widget *widget, int x, int y, int radius, Uint32 c)
{
  AG_DrawCircle (widget, x, y, radius, c);
}

void
agar_draw_circle2 (AG_Widget *widget, int x, int y, int radius, Uint32 c)
{
  AG_DrawCircle2 (widget, x, y, radius, c);
}

void
agar_draw_line (AG_Widget *widget, int x1, int y1, int x2, int y2, Uint32 c)
{
  AG_DrawLine (widget, x1, y1, x2, y2, c);
}

void
agar_draw_line2 (AG_Widget *widget, int x1, int y1, int x2, int y2, Uint32 c)
{
  AG_DrawLine2 (widget, x1, y1, x2, y2, c);
}

void
agar_draw_line_horizontal (AG_Widget *widget, int x1, int x2, int y, Uint32 c)
{
  AG_DrawLineH (widget, x1, x2, y, c);
}

void
agar_draw_line_vertical (AG_Widget *widget, int x, int y1, int y2, Uint32 c)
{
  AG_DrawLineV (widget, x, y1, y2, c);
}

void
agar_draw_rect_outline (AG_Widget *widget, AG_Rect r, Uint32 c)
{
  AG_DrawRectOutline (widget, r, c);
}

void
agar_draw_rect_filled (AG_Widget *widget, AG_Rect r, Uint32 c)
{
  AG_DrawRectFilled (widget, r, c);
}
