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
agar_widget_put_pixel_rgb (AG_Widget *widget, int x, int y, Uint8 red, Uint8 green, Uint8 blue)
{
  AG_WidgetPutPixelRGB (widget, x, y, red, green, blue);
}

void
agar_widget_blend_pixel_32 (AG_Widget *widget, int x, int y, Uint32 pixel, AG_BlendFn blendFn)
{
  AG_WidgetBlendPixel32 (widget, x, y, pixel, blendFn);
}

/* bindings */

void
agar_gui_widget_bind_pointer (AG_Widget *w, const char *binding, void **p)
{
  AG_WidgetBindPointer (w, binding, p);
}

void
agar_gui_widget_bind_property (AG_Widget *w, const char *binding,
  AG_Object *obj, const char *prop_name)
{
  AG_WidgetBindProp (w, binding, obj, prop_name);
}

void
agar_gui_widget_bind_boolean (AG_Widget *w, const char *binding, int *var)
{
  AG_WidgetBindBool (w, binding, var);
}

void
agar_gui_widget_bind_integer (AG_Widget *w, const char *binding, int *var)
{
  AG_WidgetBindInt (w, binding, var);
}

void
agar_gui_widget_bind_unsigned (AG_Widget *w, const char *binding, unsigned *var)
{
  AG_WidgetBindUint (w, binding, var);
}

void
agar_gui_widget_bind_float (AG_Widget *w, const char *binding, float *var)
{
  AG_WidgetBindFloat (w, binding, var);
}

void
agar_gui_widget_bind_double (AG_Widget *w, const char *binding, double *var)
{
  AG_WidgetBindDouble (w, binding, var);
}

void
agar_gui_widget_bind_uint8 (AG_Widget *w, const char *binding,
  Uint8 *val)
{
  AG_WidgetBindUint8 (w, binding, val);
}

void
agar_gui_widget_bind_int8 (AG_Widget *w, const char *binding,
  Sint8 *val)
{
  AG_WidgetBindSint8 (w, binding, val);
}

void
agar_gui_widget_bind_flag8 (AG_Widget *w, const char *binding,
  Uint8 *val, Uint8 bitmask)
{
  AG_WidgetBindFlag8 (w, binding, val, bitmask);
}

void
agar_gui_widget_bind_uint16 (AG_Widget *w, const char *binding,
  Uint16 *val)
{
  AG_WidgetBindUint16 (w, binding, val);
}

void
agar_gui_widget_bind_int16 (AG_Widget *w, const char *binding,
  Sint16 *val)
{
  AG_WidgetBindSint16 (w, binding, val);
}

void
agar_gui_widget_bind_flag16 (AG_Widget *w, const char *binding,
  Uint16 *val, Uint16 bitmask)
{
  AG_WidgetBindFlag16 (w, binding, val, bitmask);
}

void
agar_gui_widget_bind_uint32 (AG_Widget *w, const char *binding,
  Uint32 *val)
{
  AG_WidgetBindUint32 (w, binding, val);
}

void
agar_gui_widget_bind_int32 (AG_Widget *w, const char *binding,
  Sint32 *val)
{
  AG_WidgetBindSint32 (w, binding, val);
}

void
agar_gui_widget_bind_flag32 (AG_Widget *w, const char *binding,
  Uint32 *val, Uint32 bitmask)
{
  AG_WidgetBindFlag32 (w, binding, val, bitmask);
}

void
agar_gui_widget_bind_uint64 (AG_Widget *w, const char *binding,
  Uint64 *val)
{
  AG_WidgetBindUint64 (w, binding, val);
}

void
agar_gui_widget_bind_int64 (AG_Widget *w, const char *binding,
  Sint64 *val)
{
  AG_WidgetBindSint64 (w, binding, val);
}

void *
agar_gui_widget_get_pointer (AG_Widget *w, const char *binding)
{
  return AG_WidgetPointer (w, binding);
}

int
agar_gui_widget_get_boolean (AG_Widget *w, const char *binding)
{
  return AG_WidgetBool (w, binding);
}

int
agar_gui_widget_get_integer (AG_Widget *w, const char *binding)
{
  return AG_WidgetInt (w, binding);
}

unsigned int
agar_gui_widget_get_unsigned (AG_Widget *w, const char *binding)
{
  return AG_WidgetUint (w, binding);
}

float
agar_gui_widget_get_float (AG_Widget *w, const char *binding)
{
  return AG_WidgetFloat (w, binding);
}

double
agar_gui_widget_get_double (AG_Widget *w, const char *binding)
{
  return AG_WidgetDouble (w, binding);
}

Uint8
agar_gui_widget_get_uint8 (AG_Widget *w, const char *binding)
{
  return AG_WidgetUint8 (w, binding);
}

Sint8
agar_gui_widget_get_int8 (AG_Widget *w, const char *binding)
{
  return AG_WidgetSint8 (w, binding);
}

Uint16
agar_gui_widget_get_uint16 (AG_Widget *w, const char *binding)
{
  return AG_WidgetUint16 (w, binding);
}

Sint16
agar_gui_widget_get_int16 (AG_Widget *w, const char *binding)
{
  return AG_WidgetSint16 (w, binding);
}

Uint32
agar_gui_widget_get_uint32 (AG_Widget *w, const char *binding)
{
  return AG_WidgetUint32 (w, binding);
}

Sint32
agar_gui_widget_get_int32 (AG_Widget *w, const char *binding)
{
  return AG_WidgetSint32 (w, binding);
}

Uint64
agar_gui_widget_get_uint64 (AG_Widget *w, const char *binding)
{
  return AG_WidgetUint64 (w, binding);
}

Sint64
agar_gui_widget_get_int64 (AG_Widget *w, const char *binding)
{
  return AG_WidgetSint64 (w, binding);
}

void
agar_gui_widget_set_pointer (AG_Widget *w, const char *binding, void *val)
{
  AG_WidgetSetPointer (w, binding, val);
}

void
agar_gui_widget_set_boolean (AG_Widget *w, const char *binding, int val)
{
  AG_WidgetSetBool (w, binding, val);
}

void
agar_gui_widget_set_integer (AG_Widget *w, const char *binding, int val)
{
  AG_WidgetSetInt (w, binding, val);
}

void
agar_gui_widget_set_unsigned (AG_Widget *w, const char *binding, unsigned int val)
{
  AG_WidgetSetUint (w, binding, val);
}

void
agar_gui_widget_set_float (AG_Widget *w, const char *binding, float val)
{
  AG_WidgetSetFloat (w, binding, val);
}

void
agar_gui_widget_set_double (AG_Widget *w, const char *binding, double val)
{
  AG_WidgetSetDouble (w, binding, val);
}

void
agar_gui_widget_set_uint8 (AG_Widget *w, const char *binding,
  Uint8 val)
{
  AG_WidgetSetUint8 (w, binding, val);
}

void
agar_gui_widget_set_int8 (AG_Widget *w, const char *binding,
  Sint8 val)
{
  AG_WidgetSetSint8 (w, binding, val);
}

void
agar_gui_widget_set_uint16 (AG_Widget *w, const char *binding,
  Uint16 val)
{
  AG_WidgetSetUint16 (w, binding, val);
}

void
agar_gui_widget_set_int16 (AG_Widget *w, const char *binding,
  Sint16 val)
{
  AG_WidgetSetSint16 (w, binding, val);
}

void
agar_gui_widget_set_uint32 (AG_Widget *w, const char *binding,
  Uint32 val)
{
  AG_WidgetSetUint32 (w, binding, val);
}

void
agar_gui_widget_set_int32 (AG_Widget *w, const char *binding,
  Sint32 val)
{
  AG_WidgetSetSint32 (w, binding, val);
}

void
agar_gui_widget_set_uint64 (AG_Widget *w, const char *binding,
  Uint64 val)
{
  AG_WidgetSetUint64 (w, binding, val);
}

void
agar_gui_widget_set_int64 (AG_Widget *w, const char *binding,
  Sint64 val)
{
  AG_WidgetSetSint64 (w, binding, val);
}

