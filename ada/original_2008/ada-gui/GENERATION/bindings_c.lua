#!/usr/bin/env lua

local ts = tostring

io.write ([[
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

]])

for _, value in pairs ({8, 16, 32}) do
  io.write ([[
void
agar_gui_widget_bind_uint]]..ts(value)..[[ (AG_Widget *w, const char *binding,
  Uint]]..ts(value)..[[ *val)
{
  AG_BindUint]]..ts(value)..[[ (w, binding, val);
}

void
agar_gui_widget_bind_int]]..ts(value)..[[ (AG_Widget *w, const char *binding,
  Sint]]..ts(value)..[[ *val)
{
  AG_BindSint]]..ts(value)..[[ (w, binding, val);
}

void
agar_gui_widget_bind_flag]]..ts(value)..[[ (AG_Widget *w, const char *binding,
  Uint]]..ts(value)..[[ *val, Uint]]..ts(value)..[[ bitmask)
{
  AG_BindFlag]]..ts(value)..[[ (w, binding, val, bitmask);
}

]])
end

io.write ([[
void *
agar_gui_widget_get_pointer (AG_Widget *w, const char *binding)
{
  return AG_WidgetPointer (w, binding);
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

]])

for _, value in pairs ({8, 16, 32}) do
  io.write ([[
Uint]]..ts(value)..[[

agar_gui_widget_get_uint]]..ts(value)..[[ (AG_Widget *w, const char *binding)
{
  return AG_GetUint]]..ts(value)..[[ (w, binding);
}

Sint]]..ts(value)..[[

agar_gui_widget_get_int]]..ts(value)..[[ (AG_Widget *w, const char *binding)
{
  return AG_GetSint]]..ts(value)..[[ (w, binding);
}

]])
end

io.write ([[
void
agar_gui_widget_set_pointer (AG_Widget *w, const char *binding, void *val)
{
  AG_SetPointer (w, binding, val);
}

void
agar_gui_widget_set_boolean (AG_Widget *w, const char *binding, int val)
{
  AG_SetInt (w, binding, val);
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

]])

for _, value in pairs ({8, 16, 32}) do
  io.write ([[
void
agar_gui_widget_set_uint]]..ts(value)..[[ (AG_Widget *w, const char *binding,
  Uint]]..ts(value)..[[ val)
{
  AG_SetUint]]..ts(value)..[[ (w, binding, val);
}

void
agar_gui_widget_set_int]]..ts(value)..[[ (AG_Widget *w, const char *binding,
  Sint]]..ts(value)..[[ val)
{
  AG_SetSint]]..ts(value)..[[ (w, binding, val);
}

]])
end
