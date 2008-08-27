#include <agar/core.h>
#include <agar/gui.h>

#include <stdio.h>
#include <string.h>

struct {
  const char *type_name;
  unsigned int type_size;
} types[] = {
  /* auto generated - do not edit */
  { "AG_BlendFn", sizeof (AG_BlendFn) },
  { "enum ag_blend_func", sizeof (enum ag_blend_func) },
  { "AG_Color", sizeof (AG_Color) },
  { "struct ag_color", sizeof (struct ag_color) },
  { "AG_PixelFormat *", sizeof (AG_PixelFormat *) },
  { "AG_PixelFormat", sizeof (AG_PixelFormat) },
  { "AG_Point", sizeof (AG_Point) },
  { "struct ag_point", sizeof (struct ag_point) },
  { "AG_Rect *", sizeof (AG_Rect *) },
  { "struct ag_rect *", sizeof (struct ag_rect *) },
  { "AG_Rect", sizeof (AG_Rect) },
  { "struct ag_rect", sizeof (struct ag_rect) },
  { "AG_Style *", sizeof (AG_Style *) },
  { "struct ag_style *", sizeof (struct ag_style *) },
  { "AG_Style", sizeof (AG_Style) },
  { "struct ag_style", sizeof (struct ag_style) },
  { "AG_Surface *", sizeof (AG_Surface *) },
  { "AG_Surface", sizeof (AG_Surface) },
  { "enum ag_font_type", sizeof (enum ag_font_type) },
  { "enum ag_text_justify", sizeof (enum ag_text_justify) },
  { "enum ag_text_msg_title", sizeof (enum ag_text_msg_title) },
  { "enum ag_text_valign", sizeof (enum ag_text_valign) },
  { "AG_Unit", sizeof (AG_Unit) },
  { "struct ag_unit", sizeof (struct ag_unit) },
  { "AG_Display", sizeof (AG_Display) },
  { "struct ag_display", sizeof (struct ag_display) },
  { "AG_WidgetBinding *", sizeof (AG_WidgetBinding *) },
  { "struct ag_widget_binding *", sizeof (struct ag_widget_binding *) },
  { "AG_WidgetBinding", sizeof (AG_WidgetBinding) },
  { "struct ag_widget_binding", sizeof (struct ag_widget_binding) },
  { "AG_WidgetBindingType", sizeof (AG_WidgetBindingType) },
  { "enum ag_widget_binding_type", sizeof (enum ag_widget_binding_type) },
  { "AG_WidgetClass *", sizeof (AG_WidgetClass *) },
  { "struct ag_widget_class *", sizeof (struct ag_widget_class *) },
  { "AG_WidgetClass", sizeof (AG_WidgetClass) },
  { "struct ag_widget_class", sizeof (struct ag_widget_class) },
  { "AG_FlagDescr *", sizeof (AG_FlagDescr *) },
  { "struct ag_flag_descr *", sizeof (struct ag_flag_descr *) },
  { "AG_FlagDescr", sizeof (AG_FlagDescr) },
  { "struct ag_flag_descr", sizeof (struct ag_flag_descr) },
  { "AG_SizeReq *", sizeof (AG_SizeReq *) },
  { "struct ag_size_req *", sizeof (struct ag_size_req *) },
  { "AG_SizeReq", sizeof (AG_SizeReq) },
  { "struct ag_size_req", sizeof (struct ag_size_req) },
  { "enum ag_widget_sizespec", sizeof (enum ag_widget_sizespec) },
  { "AG_Widget *", sizeof (AG_Widget *) },
  { "struct ag_widget *", sizeof (struct ag_widget *) },
  { "AG_Widget", sizeof (AG_Widget) },
  { "struct ag_widget", sizeof (struct ag_widget) },
  { "enum ag_window_alignment", sizeof (enum ag_window_alignment) },
  { "enum ag_window_close_action", sizeof (enum ag_window_close_action) },
  { "AG_Window *", sizeof (AG_Window *) },
  { "struct ag_window *", sizeof (struct ag_window *) },
  { "AG_Window", sizeof (AG_Window) },
  { "struct ag_window", sizeof (struct ag_window) },
};
const unsigned int types_size = sizeof (types) / sizeof (types[0]);

void
find (const char *name)
{
  unsigned int pos;
  for (pos = 0; pos < types_size; ++pos) {
    if (strcmp (types[pos].type_name, name) == 0) {
      printf ("%u\n", types[pos].type_size * 8);
      return;
    }
  }
  fprintf (stderr, "fatal: unknown C type\n");
  exit (112);
}

int
main (int argc, char *argv[])
{
  if (argc != 2) exit (111);
  find (argv[1]);
  return 0;
}
