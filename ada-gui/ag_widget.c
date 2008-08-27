#include <agar/core.h>
#include <agar/gui.h>

void
agar_widget_expand (AG_Widget *widget)
{
  AG_Expand (widget);
}

void
agar_widget_expand_horizontal (AG_Widget *widget)
{
  AG_ExpandHoriz (widget);
}

void
agar_widget_expand_vertical (AG_Widget *widget)
{
  AG_ExpandVert (widget);
}

void
agar_widget_enable (AG_Widget *widget)
{
  AG_WidgetEnable (widget);
}

void
agar_widget_disable (AG_Widget *widget)
{
  AG_WidgetDisable (widget);
}

int
agar_widget_enabled (AG_Widget *widget)
{
  return AG_WidgetEnabled (widget);
}

int
agar_widget_disabled (AG_Widget *widget)
{
  return AG_WidgetDisabled (widget);
}

int
agar_widget_focused (AG_Widget *widget)
{
  return AG_WidgetFocused (widget);
}

int
agar_widget_area (AG_Widget *widget, int x, int y)
{
  return AG_WidgetArea (widget, x, y);
}

int
agar_widget_relative_area (AG_Widget *widget, int x, int y)
{
  return AG_WidgetRelativeArea (widget, x, y);
}

void
agar_widget_unmap_surface (AG_Widget *widget, int surface_id)
{
  AG_WidgetUnmapSurface (widget, surface_id);
}

void
agar_widget_update_surface (AG_Widget *widget, int surface_id)
{
  AG_WidgetUpdateSurface (widget, surface_id);
}

void
agar_widget_blit_surface (AG_Widget *widget, int surface_id, int x, int y)
{
  AG_WidgetBlitSurface (widget, surface_id, x, y);
}

void
agar_widget_put_pixel32 (AG_Widget *widget, int x, int y, Uint32 color)
{
  AG_WidgetPutPixel32 (widget, x, y, color);
}

void
agar_widget_put_pixel32_or_clip (AG_Widget *widget, int x, int y, Uint32 color)
{
  AG_WidgetPutPixel32OrClip (widget, x, y, color);
}

void
agar_widget_put_pixel_rgb (AG_Widget *widget, int x, int y, Uint8 red, Uint8 green, Uint8 blue)
{
  AG_WidgetPutPixelRGB (widget, x, y, red, green, blue);
}

void
agar_widget_put_pixel_rgb_or_clip (AG_Widget *widget, int x, int y, Uint8 red, Uint8 green, Uint8 blue)
{
  AG_WidgetPutPixelRGBOrClip (widget, x, y, red, green, blue);
}

void
agar_widget_blend_pixel_32 (AG_Widget *widget, int x, int y, Uint32 pixel, AG_BlendFn blendFn)
{
  AG_WidgetBlendPixel32 (widget, x, y, pixel, blendFn);
}
