with ada.text_io;
with ada.command_line;

with agar.gui.colors;
with agar.gui.pixelformat;
with agar.gui.point;
with agar.gui.rect;
with agar.gui.style;
with agar.gui.surface;
with agar.gui.text;
with agar.gui.unit;
with agar.gui.view;
with agar.gui.widget.box;
with agar.gui.widget;
with agar.gui.window;
with agar.gui;
with agar;

procedure ada_size is
  package io renames ada.text_io;
  package cmdline renames ada.command_line;

  -- auto generated - do not edit
  agar_gui_colors_blend_func_t : aliased string := "agar.gui.colors.blend_func_t";
  agar_gui_colors_color_t : aliased string := "agar.gui.colors.color_t";
  agar_gui_pixelformat_pixel_format_access_t : aliased string := "agar.gui.pixelformat.pixel_format_access_t";
  agar_gui_pixelformat_pixel_format_t : aliased string := "agar.gui.pixelformat.pixel_format_t";
  agar_gui_point_point_t : aliased string := "agar.gui.point.point_t";
  agar_gui_rect_rect_access_t : aliased string := "agar.gui.rect.rect_access_t";
  agar_gui_rect_rect_t : aliased string := "agar.gui.rect.rect_t";
  agar_gui_style_style_access_t : aliased string := "agar.gui.style.style_access_t";
  agar_gui_style_style_t : aliased string := "agar.gui.style.style_t";
  agar_gui_surface_surface_access_t : aliased string := "agar.gui.surface.surface_access_t";
  agar_gui_surface_surface_t : aliased string := "agar.gui.surface.surface_t";
  agar_gui_text_font_type_t : aliased string := "agar.gui.text.font_type_t";
  agar_gui_text_justify_t : aliased string := "agar.gui.text.justify_t";
  agar_gui_text_msg_title_t : aliased string := "agar.gui.text.msg_title_t";
  agar_gui_text_valign_t : aliased string := "agar.gui.text.valign_t";
  agar_gui_unit_unit_t : aliased string := "agar.gui.unit.unit_t";
  agar_gui_view_display_t : aliased string := "agar.gui.view.display_t";
  agar_gui_widget_binding_access_t : aliased string := "agar.gui.widget.binding_access_t";
  agar_gui_widget_binding_t : aliased string := "agar.gui.widget.binding_t";
  agar_gui_widget_binding_type_t : aliased string := "agar.gui.widget.binding_type_t";
  agar_gui_widget_box_box_access_t : aliased string := "agar.gui.widget.box.box_access_t";
  agar_gui_widget_box_box_t : aliased string := "agar.gui.widget.box.box_t";
  agar_gui_widget_box_type_t : aliased string := "agar.gui.widget.box.type_t";
  agar_gui_widget_class_access_t : aliased string := "agar.gui.widget.class_access_t";
  agar_gui_widget_class_t : aliased string := "agar.gui.widget.class_t";
  agar_gui_widget_flag_descr_access_t : aliased string := "agar.gui.widget.flag_descr_access_t";
  agar_gui_widget_flag_descr_t : aliased string := "agar.gui.widget.flag_descr_t";
  agar_gui_widget_size_req_access_t : aliased string := "agar.gui.widget.size_req_access_t";
  agar_gui_widget_size_req_t : aliased string := "agar.gui.widget.size_req_t";
  agar_gui_widget_size_spec_t : aliased string := "agar.gui.widget.size_spec_t";
  agar_gui_widget_widget_access_t : aliased string := "agar.gui.widget.widget_access_t";
  agar_gui_widget_widget_t : aliased string := "agar.gui.widget.widget_t";
  agar_gui_window_alignment_t : aliased string := "agar.gui.window.alignment_t";
  agar_gui_window_close_action_t : aliased string := "agar.gui.window.close_action_t";
  agar_gui_window_window_access_t : aliased string := "agar.gui.window.window_access_t";
  agar_gui_window_window_t : aliased string := "agar.gui.window.window_t";

  type type_t is record
    name : access string;
    size : natural;
  end record;
  type type_lookup_t is array (natural range <>) of type_t;

  types : aliased constant type_lookup_t := (
    (agar_gui_colors_blend_func_t'access, agar.gui.colors.blend_func_t'size),
    (agar_gui_colors_color_t'access, agar.gui.colors.color_t'size),
    (agar_gui_pixelformat_pixel_format_access_t'access, agar.gui.pixelformat.pixel_format_access_t'size),
    (agar_gui_pixelformat_pixel_format_t'access, agar.gui.pixelformat.pixel_format_t'size),
    (agar_gui_point_point_t'access, agar.gui.point.point_t'size),
    (agar_gui_rect_rect_access_t'access, agar.gui.rect.rect_access_t'size),
    (agar_gui_rect_rect_t'access, agar.gui.rect.rect_t'size),
    (agar_gui_style_style_access_t'access, agar.gui.style.style_access_t'size),
    (agar_gui_style_style_t'access, agar.gui.style.style_t'size),
    (agar_gui_surface_surface_access_t'access, agar.gui.surface.surface_access_t'size),
    (agar_gui_surface_surface_t'access, agar.gui.surface.surface_t'size),
    (agar_gui_text_font_type_t'access, agar.gui.text.font_type_t'size),
    (agar_gui_text_justify_t'access, agar.gui.text.justify_t'size),
    (agar_gui_text_msg_title_t'access, agar.gui.text.msg_title_t'size),
    (agar_gui_text_valign_t'access, agar.gui.text.valign_t'size),
    (agar_gui_unit_unit_t'access, agar.gui.unit.unit_t'size),
    (agar_gui_view_display_t'access, agar.gui.view.display_t'size),
    (agar_gui_widget_binding_access_t'access, agar.gui.widget.binding_access_t'size),
    (agar_gui_widget_binding_t'access, agar.gui.widget.binding_t'size),
    (agar_gui_widget_binding_type_t'access, agar.gui.widget.binding_type_t'size),
    (agar_gui_widget_box_box_access_t'access, agar.gui.widget.box.box_access_t'size),
    (agar_gui_widget_box_box_t'access, agar.gui.widget.box.box_t'size),
    (agar_gui_widget_box_type_t'access, agar.gui.widget.box.type_t'size),
    (agar_gui_widget_class_access_t'access, agar.gui.widget.class_access_t'size),
    (agar_gui_widget_class_t'access, agar.gui.widget.class_t'size),
    (agar_gui_widget_flag_descr_access_t'access, agar.gui.widget.flag_descr_access_t'size),
    (agar_gui_widget_flag_descr_t'access, agar.gui.widget.flag_descr_t'size),
    (agar_gui_widget_size_req_access_t'access, agar.gui.widget.size_req_access_t'size),
    (agar_gui_widget_size_req_t'access, agar.gui.widget.size_req_t'size),
    (agar_gui_widget_size_spec_t'access, agar.gui.widget.size_spec_t'size),
    (agar_gui_widget_widget_access_t'access, agar.gui.widget.widget_access_t'size),
    (agar_gui_widget_widget_t'access, agar.gui.widget.widget_t'size),
    (agar_gui_window_alignment_t'access, agar.gui.window.alignment_t'size),
    (agar_gui_window_close_action_t'access, agar.gui.window.close_action_t'size),
    (agar_gui_window_window_access_t'access, agar.gui.window.window_access_t'size),
    (agar_gui_window_window_t'access, agar.gui.window.window_t'size)
  );

  procedure find (name : string) is
  begin
    for index in types'range loop
      if types (index).name.all = name then
        io.put_line (natural'image (types (index).size));
        return;
      end if;
    end loop;
    raise program_error with "fatal: unknown ada type";
  end find;

begin
  if cmdline.argument_count /= 1 then
    raise program_error with "fatal: incorrect number of args";
  end if;
  find (cmdline.argument (1));
end ada_size;
