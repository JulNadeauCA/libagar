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
with agar.gui.widget.button;
with agar.gui.widget.checkbox;
with agar.gui.widget.combo;
with agar.gui.widget.console;
with agar.gui.widget.editable;
with agar.gui.widget.file_dialog;
with agar.gui.widget.label;
with agar.gui.widget.menu;
with agar.gui.widget.pane;
with agar.gui.widget.scrollbar;
with agar.gui.widget.textbox;
with agar.gui.widget.tlist;
with agar.gui.widget.toolbar;
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
  agar_gui_text_font_access_t : aliased string := "agar.gui.text.font_access_t";
  agar_gui_text_font_t : aliased string := "agar.gui.text.font_t";
  agar_gui_text_font_type_t : aliased string := "agar.gui.text.font_type_t";
  agar_gui_text_glyph_access_t : aliased string := "agar.gui.text.glyph_access_t";
  agar_gui_text_glyph_t : aliased string := "agar.gui.text.glyph_t";
  agar_gui_text_justify_t : aliased string := "agar.gui.text.justify_t";
  agar_gui_text_metrics_access_t : aliased string := "agar.gui.text.metrics_access_t";
  agar_gui_text_metrics_t : aliased string := "agar.gui.text.metrics_t";
  agar_gui_text_msg_title_t : aliased string := "agar.gui.text.msg_title_t";
  agar_gui_text_state_access_t : aliased string := "agar.gui.text.state_access_t";
  agar_gui_text_state_t : aliased string := "agar.gui.text.state_t";
  agar_gui_text_static_font_access_t : aliased string := "agar.gui.text.static_font_access_t";
  agar_gui_text_static_font_t : aliased string := "agar.gui.text.static_font_t";
  agar_gui_text_valign_t : aliased string := "agar.gui.text.valign_t";
  agar_gui_unit_unit_t : aliased string := "agar.gui.unit.unit_t";
  agar_gui_view_display_t : aliased string := "agar.gui.view.display_t";
  agar_gui_widget_binding_access_t : aliased string := "agar.gui.widget.binding_access_t";
  agar_gui_widget_binding_t : aliased string := "agar.gui.widget.binding_t";
  agar_gui_widget_binding_type_t : aliased string := "agar.gui.widget.binding_type_t";
  agar_gui_widget_box_box_access_t : aliased string := "agar.gui.widget.box.box_access_t";
  agar_gui_widget_box_box_t : aliased string := "agar.gui.widget.box.box_t";
  agar_gui_widget_box_type_t : aliased string := "agar.gui.widget.box.type_t";
  agar_gui_widget_button_button_access_t : aliased string := "agar.gui.widget.button.button_access_t";
  agar_gui_widget_button_button_t : aliased string := "agar.gui.widget.button.button_t";
  agar_gui_widget_checkbox_checkbox_access_t : aliased string := "agar.gui.widget.checkbox.checkbox_access_t";
  agar_gui_widget_checkbox_checkbox_t : aliased string := "agar.gui.widget.checkbox.checkbox_t";
  agar_gui_widget_class_access_t : aliased string := "agar.gui.widget.class_access_t";
  agar_gui_widget_class_t : aliased string := "agar.gui.widget.class_t";
  agar_gui_widget_combo_combo_access_t : aliased string := "agar.gui.widget.combo.combo_access_t";
  agar_gui_widget_combo_combo_t : aliased string := "agar.gui.widget.combo.combo_t";
  agar_gui_widget_console_console_access_t : aliased string := "agar.gui.widget.console.console_access_t";
  agar_gui_widget_console_console_t : aliased string := "agar.gui.widget.console.console_t";
  agar_gui_widget_console_line_access_t : aliased string := "agar.gui.widget.console.line_access_t";
  agar_gui_widget_console_line_t : aliased string := "agar.gui.widget.console.line_t";
  agar_gui_widget_editable_editable_access_t : aliased string := "agar.gui.widget.editable.editable_access_t";
  agar_gui_widget_editable_editable_t : aliased string := "agar.gui.widget.editable.editable_t";
  agar_gui_widget_editable_encoding_t : aliased string := "agar.gui.widget.editable.encoding_t";
  agar_gui_widget_file_dialog_file_dialog_access_t : aliased string := "agar.gui.widget.file_dialog.file_dialog_access_t";
  agar_gui_widget_file_dialog_file_dialog_t : aliased string := "agar.gui.widget.file_dialog.file_dialog_t";
  agar_gui_widget_file_dialog_filetype_access_t : aliased string := "agar.gui.widget.file_dialog.filetype_access_t";
  agar_gui_widget_file_dialog_filetype_t : aliased string := "agar.gui.widget.file_dialog.filetype_t";
  agar_gui_widget_file_dialog_option_access_t : aliased string := "agar.gui.widget.file_dialog.option_access_t";
  agar_gui_widget_file_dialog_option_t : aliased string := "agar.gui.widget.file_dialog.option_t";
  agar_gui_widget_file_dialog_option_type_t : aliased string := "agar.gui.widget.file_dialog.option_type_t";
  agar_gui_widget_flag_descr_access_t : aliased string := "agar.gui.widget.flag_descr_access_t";
  agar_gui_widget_flag_descr_t : aliased string := "agar.gui.widget.flag_descr_t";
  agar_gui_widget_label_flag_t : aliased string := "agar.gui.widget.label.flag_t";
  agar_gui_widget_label_format_func_t : aliased string := "agar.gui.widget.label.format_func_t";
  agar_gui_widget_label_format_spec_access_t : aliased string := "agar.gui.widget.label.format_spec_access_t";
  agar_gui_widget_label_format_spec_t : aliased string := "agar.gui.widget.label.format_spec_t";
  agar_gui_widget_label_label_access_t : aliased string := "agar.gui.widget.label.label_access_t";
  agar_gui_widget_label_label_t : aliased string := "agar.gui.widget.label.label_t";
  agar_gui_widget_label_type_t : aliased string := "agar.gui.widget.label.type_t";
  agar_gui_widget_menu_binding_t : aliased string := "agar.gui.widget.menu.binding_t";
  agar_gui_widget_menu_item_access_t : aliased string := "agar.gui.widget.menu.item_access_t";
  agar_gui_widget_menu_item_t : aliased string := "agar.gui.widget.menu.item_t";
  agar_gui_widget_menu_menu_access_t : aliased string := "agar.gui.widget.menu.menu_access_t";
  agar_gui_widget_menu_menu_t : aliased string := "agar.gui.widget.menu.menu_t";
  agar_gui_widget_menu_view_access_t : aliased string := "agar.gui.widget.menu.view_access_t";
  agar_gui_widget_menu_view_t : aliased string := "agar.gui.widget.menu.view_t";
  agar_gui_widget_pane_pane_access_t : aliased string := "agar.gui.widget.pane.pane_access_t";
  agar_gui_widget_pane_pane_t : aliased string := "agar.gui.widget.pane.pane_t";
  agar_gui_widget_pane_type_t : aliased string := "agar.gui.widget.pane.type_t";
  agar_gui_widget_scrollbar_button_t : aliased string := "agar.gui.widget.scrollbar.button_t";
  agar_gui_widget_scrollbar_scrollbar_access_t : aliased string := "agar.gui.widget.scrollbar.scrollbar_access_t";
  agar_gui_widget_scrollbar_scrollbar_t : aliased string := "agar.gui.widget.scrollbar.scrollbar_t";
  agar_gui_widget_scrollbar_type_t : aliased string := "agar.gui.widget.scrollbar.type_t";
  agar_gui_widget_size_req_access_t : aliased string := "agar.gui.widget.size_req_access_t";
  agar_gui_widget_size_req_t : aliased string := "agar.gui.widget.size_req_t";
  agar_gui_widget_size_spec_t : aliased string := "agar.gui.widget.size_spec_t";
  agar_gui_widget_textbox_textbox_access_t : aliased string := "agar.gui.widget.textbox.textbox_access_t";
  agar_gui_widget_textbox_textbox_t : aliased string := "agar.gui.widget.textbox.textbox_t";
  agar_gui_widget_tlist_item_access_t : aliased string := "agar.gui.widget.tlist.item_access_t";
  agar_gui_widget_tlist_item_t : aliased string := "agar.gui.widget.tlist.item_t";
  agar_gui_widget_tlist_popup_access_t : aliased string := "agar.gui.widget.tlist.popup_access_t";
  agar_gui_widget_tlist_popup_t : aliased string := "agar.gui.widget.tlist.popup_t";
  agar_gui_widget_tlist_tlist_access_t : aliased string := "agar.gui.widget.tlist.tlist_access_t";
  agar_gui_widget_tlist_tlist_t : aliased string := "agar.gui.widget.tlist.tlist_t";
  agar_gui_widget_toolbar_toolbar_access_t : aliased string := "agar.gui.widget.toolbar.toolbar_access_t";
  agar_gui_widget_toolbar_toolbar_t : aliased string := "agar.gui.widget.toolbar.toolbar_t";
  agar_gui_widget_toolbar_type_t : aliased string := "agar.gui.widget.toolbar.type_t";
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
    (agar_gui_text_font_access_t'access, agar.gui.text.font_access_t'size),
    (agar_gui_text_font_t'access, agar.gui.text.font_t'size),
    (agar_gui_text_font_type_t'access, agar.gui.text.font_type_t'size),
    (agar_gui_text_glyph_access_t'access, agar.gui.text.glyph_access_t'size),
    (agar_gui_text_glyph_t'access, agar.gui.text.glyph_t'size),
    (agar_gui_text_justify_t'access, agar.gui.text.justify_t'size),
    (agar_gui_text_metrics_access_t'access, agar.gui.text.metrics_access_t'size),
    (agar_gui_text_metrics_t'access, agar.gui.text.metrics_t'size),
    (agar_gui_text_msg_title_t'access, agar.gui.text.msg_title_t'size),
    (agar_gui_text_state_access_t'access, agar.gui.text.state_access_t'size),
    (agar_gui_text_state_t'access, agar.gui.text.state_t'size),
    (agar_gui_text_static_font_access_t'access, agar.gui.text.static_font_access_t'size),
    (agar_gui_text_static_font_t'access, agar.gui.text.static_font_t'size),
    (agar_gui_text_valign_t'access, agar.gui.text.valign_t'size),
    (agar_gui_unit_unit_t'access, agar.gui.unit.unit_t'size),
    (agar_gui_view_display_t'access, agar.gui.view.display_t'size),
    (agar_gui_widget_binding_access_t'access, agar.gui.widget.binding_access_t'size),
    (agar_gui_widget_binding_t'access, agar.gui.widget.binding_t'size),
    (agar_gui_widget_binding_type_t'access, agar.gui.widget.binding_type_t'size),
    (agar_gui_widget_box_box_access_t'access, agar.gui.widget.box.box_access_t'size),
    (agar_gui_widget_box_box_t'access, agar.gui.widget.box.box_t'size),
    (agar_gui_widget_box_type_t'access, agar.gui.widget.box.type_t'size),
    (agar_gui_widget_button_button_access_t'access, agar.gui.widget.button.button_access_t'size),
    (agar_gui_widget_button_button_t'access, agar.gui.widget.button.button_t'size),
    (agar_gui_widget_checkbox_checkbox_access_t'access, agar.gui.widget.checkbox.checkbox_access_t'size),
    (agar_gui_widget_checkbox_checkbox_t'access, agar.gui.widget.checkbox.checkbox_t'size),
    (agar_gui_widget_class_access_t'access, agar.gui.widget.class_access_t'size),
    (agar_gui_widget_class_t'access, agar.gui.widget.class_t'size),
    (agar_gui_widget_combo_combo_access_t'access, agar.gui.widget.combo.combo_access_t'size),
    (agar_gui_widget_combo_combo_t'access, agar.gui.widget.combo.combo_t'size),
    (agar_gui_widget_console_console_access_t'access, agar.gui.widget.console.console_access_t'size),
    (agar_gui_widget_console_console_t'access, agar.gui.widget.console.console_t'size),
    (agar_gui_widget_console_line_access_t'access, agar.gui.widget.console.line_access_t'size),
    (agar_gui_widget_console_line_t'access, agar.gui.widget.console.line_t'size),
    (agar_gui_widget_editable_editable_access_t'access, agar.gui.widget.editable.editable_access_t'size),
    (agar_gui_widget_editable_editable_t'access, agar.gui.widget.editable.editable_t'size),
    (agar_gui_widget_editable_encoding_t'access, agar.gui.widget.editable.encoding_t'size),
    (agar_gui_widget_file_dialog_file_dialog_access_t'access, agar.gui.widget.file_dialog.file_dialog_access_t'size),
    (agar_gui_widget_file_dialog_file_dialog_t'access, agar.gui.widget.file_dialog.file_dialog_t'size),
    (agar_gui_widget_file_dialog_filetype_access_t'access, agar.gui.widget.file_dialog.filetype_access_t'size),
    (agar_gui_widget_file_dialog_filetype_t'access, agar.gui.widget.file_dialog.filetype_t'size),
    (agar_gui_widget_file_dialog_option_access_t'access, agar.gui.widget.file_dialog.option_access_t'size),
    (agar_gui_widget_file_dialog_option_t'access, agar.gui.widget.file_dialog.option_t'size),
    (agar_gui_widget_file_dialog_option_type_t'access, agar.gui.widget.file_dialog.option_type_t'size),
    (agar_gui_widget_flag_descr_access_t'access, agar.gui.widget.flag_descr_access_t'size),
    (agar_gui_widget_flag_descr_t'access, agar.gui.widget.flag_descr_t'size),
    (agar_gui_widget_label_flag_t'access, agar.gui.widget.label.flag_t'size),
    (agar_gui_widget_label_format_func_t'access, agar.gui.widget.label.format_func_t'size),
    (agar_gui_widget_label_format_spec_access_t'access, agar.gui.widget.label.format_spec_access_t'size),
    (agar_gui_widget_label_format_spec_t'access, agar.gui.widget.label.format_spec_t'size),
    (agar_gui_widget_label_label_access_t'access, agar.gui.widget.label.label_access_t'size),
    (agar_gui_widget_label_label_t'access, agar.gui.widget.label.label_t'size),
    (agar_gui_widget_label_type_t'access, agar.gui.widget.label.type_t'size),
    (agar_gui_widget_menu_binding_t'access, agar.gui.widget.menu.binding_t'size),
    (agar_gui_widget_menu_item_access_t'access, agar.gui.widget.menu.item_access_t'size),
    (agar_gui_widget_menu_item_t'access, agar.gui.widget.menu.item_t'size),
    (agar_gui_widget_menu_menu_access_t'access, agar.gui.widget.menu.menu_access_t'size),
    (agar_gui_widget_menu_menu_t'access, agar.gui.widget.menu.menu_t'size),
    (agar_gui_widget_menu_view_access_t'access, agar.gui.widget.menu.view_access_t'size),
    (agar_gui_widget_menu_view_t'access, agar.gui.widget.menu.view_t'size),
    (agar_gui_widget_pane_pane_access_t'access, agar.gui.widget.pane.pane_access_t'size),
    (agar_gui_widget_pane_pane_t'access, agar.gui.widget.pane.pane_t'size),
    (agar_gui_widget_pane_type_t'access, agar.gui.widget.pane.type_t'size),
    (agar_gui_widget_scrollbar_button_t'access, agar.gui.widget.scrollbar.button_t'size),
    (agar_gui_widget_scrollbar_scrollbar_access_t'access, agar.gui.widget.scrollbar.scrollbar_access_t'size),
    (agar_gui_widget_scrollbar_scrollbar_t'access, agar.gui.widget.scrollbar.scrollbar_t'size),
    (agar_gui_widget_scrollbar_type_t'access, agar.gui.widget.scrollbar.type_t'size),
    (agar_gui_widget_size_req_access_t'access, agar.gui.widget.size_req_access_t'size),
    (agar_gui_widget_size_req_t'access, agar.gui.widget.size_req_t'size),
    (agar_gui_widget_size_spec_t'access, agar.gui.widget.size_spec_t'size),
    (agar_gui_widget_textbox_textbox_access_t'access, agar.gui.widget.textbox.textbox_access_t'size),
    (agar_gui_widget_textbox_textbox_t'access, agar.gui.widget.textbox.textbox_t'size),
    (agar_gui_widget_tlist_item_access_t'access, agar.gui.widget.tlist.item_access_t'size),
    (agar_gui_widget_tlist_item_t'access, agar.gui.widget.tlist.item_t'size),
    (agar_gui_widget_tlist_popup_access_t'access, agar.gui.widget.tlist.popup_access_t'size),
    (agar_gui_widget_tlist_popup_t'access, agar.gui.widget.tlist.popup_t'size),
    (agar_gui_widget_tlist_tlist_access_t'access, agar.gui.widget.tlist.tlist_access_t'size),
    (agar_gui_widget_tlist_tlist_t'access, agar.gui.widget.tlist.tlist_t'size),
    (agar_gui_widget_toolbar_toolbar_access_t'access, agar.gui.widget.toolbar.toolbar_access_t'size),
    (agar_gui_widget_toolbar_toolbar_t'access, agar.gui.widget.toolbar.toolbar_t'size),
    (agar_gui_widget_toolbar_type_t'access, agar.gui.widget.toolbar.type_t'size),
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
