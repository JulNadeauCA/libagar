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
  return AG_WidgetIsFocused (widget);
}

int
agar_widget_focused_in_window (AG_Widget *widget)
{
  return AG_WidgetIsFocusedInWindow (widget);
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
agar_widget_put_pixel (AG_Widget *widget, int x, int y, AG_Color color)
{
  AG_PutPixel (widget, x, y, color);
}

void
agar_widget_put_pixel32 (AG_Widget *widget, int x, int y, Uint32 color)
{
  AG_PutPixel32 (widget, x, y, color);
}

void
agar_widget_put_pixel_rgb (AG_Widget *widget, int x, int y, Uint8 red, Uint8 green, Uint8 blue)
{
  AG_PutPixelRGB (widget, x, y, red, green, blue);
}

void
agar_widget_blend_pixel (AG_Widget *widget, int x, int y, AG_Color color, AG_BlendFn blendFn)
{
  AG_BlendPixel (widget, x, y, color, blendFn);
}

void
agar_widget_blend_pixel_32 (AG_Widget *widget, int x, int y, Uint32 pixel, AG_BlendFn blendFn)
{
  AG_BlendPixel32 (widget, x, y, pixel, blendFn);
}

/* bindings */

void
agar_gui_widget_bind_pointer (AG_Widget *w, const char *binding, void **p)
{
  AG_BindPointer (w, binding, p);
}

void
agar_gui_widget_bind_boolean (AG_Widget *w, const char *binding, int *var)
{
  AG_BindBool (w, binding, var);
}

void
agar_gui_widget_bind_integer (AG_Widget *w, const char *binding, int *var)
{
  AG_BindInt (w, binding, var);
}

void
agar_gui_widget_bind_unsigned (AG_Widget *w, const char *binding, unsigned *var)
{
  AG_BindUint (w, binding, var);
}

void
agar_gui_widget_bind_float (AG_Widget *w, const char *binding, float *var)
{
  AG_BindFloat (w, binding, var);
}

void
agar_gui_widget_bind_double (AG_Widget *w, const char *binding, double *var)
{
  AG_BindDouble (w, binding, var);
}

void
agar_gui_widget_bind_uint8 (AG_Widget *w, const char *binding,
  Uint8 *val)
{
  AG_BindUint8 (w, binding, val);
}

void
agar_gui_widget_bind_int8 (AG_Widget *w, const char *binding,
  Sint8 *val)
{
  AG_BindSint8 (w, binding, val);
}

void
agar_gui_widget_bind_flag8 (AG_Widget *w, const char *binding,
  Uint8 *val, Uint8 bitmask)
{
  AG_BindFlag8 (w, binding, val, bitmask);
}

void
agar_gui_widget_bind_uint16 (AG_Widget *w, const char *binding,
  Uint16 *val)
{
  AG_BindUint16 (w, binding, val);
}

void
agar_gui_widget_bind_int16 (AG_Widget *w, const char *binding,
  Sint16 *val)
{
  AG_BindSint16 (w, binding, val);
}

void
agar_gui_widget_bind_flag16 (AG_Widget *w, const char *binding,
  Uint16 *val, Uint16 bitmask)
{
  AG_BindFlag16 (w, binding, val, bitmask);
}

void
agar_gui_widget_bind_uint32 (AG_Widget *w, const char *binding,
  Uint32 *val)
{
  AG_BindUint32 (w, binding, val);
}

void
agar_gui_widget_bind_int32 (AG_Widget *w, const char *binding,
  Sint32 *val)
{
  AG_BindSint32 (w, binding, val);
}

void
agar_gui_widget_bind_flag32 (AG_Widget *w, const char *binding,
  Uint32 *val, Uint32 bitmask)
{
  AG_BindFlag32 (w, binding, val, bitmask);
}

void *
agar_gui_widget_get_pointer (AG_Widget *w, const char *binding)
{
  return AG_GetPointer (w, binding);
}

int
agar_gui_widget_get_boolean (AG_Widget *w, const char *binding)
{
  return AG_GetInt (w, binding);
}

int
agar_gui_widget_get_integer (AG_Widget *w, const char *binding)
{
  return AG_GetInt (w, binding);
}

unsigned int
agar_gui_widget_get_unsigned (AG_Widget *w, const char *binding)
{
  return AG_GetUint (w, binding);
}

float
agar_gui_widget_get_float (AG_Widget *w, const char *binding)
{
  return AG_GetFloat (w, binding);
}

double
agar_gui_widget_get_double (AG_Widget *w, const char *binding)
{
  return AG_GetDouble (w, binding);
}

Uint8
agar_gui_widget_get_uint8 (AG_Widget *w, const char *binding)
{
  return AG_GetUint8 (w, binding);
}

Sint8
agar_gui_widget_get_int8 (AG_Widget *w, const char *binding)
{
  return AG_GetSint8 (w, binding);
}

Uint16
agar_gui_widget_get_uint16 (AG_Widget *w, const char *binding)
{
  return AG_GetUint16 (w, binding);
}

Sint16
agar_gui_widget_get_int16 (AG_Widget *w, const char *binding)
{
  return AG_GetSint16 (w, binding);
}

Uint32
agar_gui_widget_get_uint32 (AG_Widget *w, const char *binding)
{
  return AG_GetUint32 (w, binding);
}

Sint32
agar_gui_widget_get_int32 (AG_Widget *w, const char *binding)
{
  return AG_GetSint32 (w, binding);
}

void
agar_gui_widget_set_pointer (AG_Widget *w, const char *binding, void *val)
{
  AG_SetPointer (w, binding, val);
}

void
agar_gui_widget_set_boolean (AG_Widget *w, const char *binding, int val)
{
  AG_SetBool (w, binding, val);
}

void
agar_gui_widget_set_integer (AG_Widget *w, const char *binding, int val)
{
  AG_SetInt (w, binding, val);
}

void
agar_gui_widget_set_unsigned (AG_Widget *w, const char *binding, unsigned int val)
{
  AG_SetUint (w, binding, val);
}

void
agar_gui_widget_set_float (AG_Widget *w, const char *binding, float val)
{
  AG_SetFloat (w, binding, val);
}

void
agar_gui_widget_set_double (AG_Widget *w, const char *binding, double val)
{
  AG_SetDouble (w, binding, val);
}

void
agar_gui_widget_set_uint8 (AG_Widget *w, const char *binding,
  Uint8 val)
{
  AG_SetUint8 (w, binding, val);
}

void
agar_gui_widget_set_int8 (AG_Widget *w, const char *binding,
  Sint8 val)
{
  AG_SetSint8 (w, binding, val);
}

void
agar_gui_widget_set_uint16 (AG_Widget *w, const char *binding,
  Uint16 val)
{
  AG_SetUint16 (w, binding, val);
}

void
agar_gui_widget_set_int16 (AG_Widget *w, const char *binding,
  Sint16 val)
{
  AG_SetSint16 (w, binding, val);
}

void
agar_gui_widget_set_uint32 (AG_Widget *w, const char *binding,
  Uint32 val)
{
  AG_SetUint32 (w, binding, val);
}

void
agar_gui_widget_set_int32 (AG_Widget *w, const char *binding,
  Sint32 val)
{
  AG_SetSint32 (w, binding, val);
}

