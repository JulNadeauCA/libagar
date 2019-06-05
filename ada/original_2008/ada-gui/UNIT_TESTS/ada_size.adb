-- auto generated, do not edit

with ada.text_io;
with ada.command_line;

with agar;
with agar.gui;
with agar.gui.colors;
with agar.gui.pixelformat;
with agar.gui.point;
with agar.gui.rect;
with agar.gui.style;
with agar.gui.surface;
with agar.gui.text;
with agar.gui.unit;
with agar.gui.view;
with agar.gui.widget;
with agar.gui.widget.box;
with agar.gui.widget.button;
with agar.gui.widget.checkbox;
with agar.gui.widget.combo;
with agar.gui.widget.console;
with agar.gui.widget.editable;
with agar.gui.widget.file_dialog;
with agar.gui.widget.fixed;
with agar.gui.widget.fixed_plotter;
with agar.gui.widget.graph;
with agar.gui.widget.hbox;
with agar.gui.widget.hsvpal;
with agar.gui.widget.icon;
with agar.gui.widget.label;
with agar.gui.widget.menu;
with agar.gui.widget.mpane;
with agar.gui.widget.notebook;
with agar.gui.widget.numerical;
with agar.gui.widget.pane;
with agar.gui.widget.pixmap;
with agar.gui.widget.progress_bar;
with agar.gui.widget.radio;
with agar.gui.widget.scrollbar;
with agar.gui.widget.separator;
with agar.gui.widget.slider;
with agar.gui.widget.socket;
with agar.gui.widget.table;
with agar.gui.widget.textbox;
with agar.gui.widget.titlebar;
with agar.gui.widget.tlist;
with agar.gui.widget.toolbar;
with agar.gui.widget.ucombo;
with agar.gui.widget.vbox;
with agar.gui.window;

procedure ada_size is
  package io renames ada.text_io;
  package cmdline renames ada.command_line;

  -- generic types
  type generic_t is new integer;
  type generic_access_t is access all generic_t;

  -- package instantiations
  package gen_agar_gui_widget_fixed is new agar.gui.widget.fixed
    (child_type => generic_t, child_access_type => generic_access_t);

  -- type names
  agar_gui_colors_blend_func_t : aliased string := "agar.gui.colors.blend_func_t";
  agar_gui_colors_color_t : aliased string := "agar.gui.colors.color_t";
  agar_gui_pixelformat_pixel_format_access_t : aliased string := "agar.gui.pixelformat.pixel_format_access_t";
  agar_gui_pixelformat_pixel_format_t : aliased string := "agar.gui.pixelformat.pixel_format_t";
  agar_gui_point_point_t : aliased string := "agar.gui.point.point_t";
  agar_gui_rect_rect_access_t : aliased string := "agar.gui.rect.rect_access_t";
  agar_gui_rect_rect_t : aliased string := "agar.gui.rect.rect_t";
  agar_gui_rect_rect2_access_t : aliased string := "agar.gui.rect.rect2_access_t";
  agar_gui_rect_rect2_t : aliased string := "agar.gui.rect.rect2_t";
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
  agar_gui_widget_fixed_fixed_access_t : aliased string := "agar.gui.widget.fixed.fixed_access_t";
  agar_gui_widget_fixed_fixed_t : aliased string := "agar.gui.widget.fixed.fixed_t";
  agar_gui_widget_fixed_plotter_item_access_t : aliased string := "agar.gui.widget.fixed_plotter.item_access_t";
  agar_gui_widget_fixed_plotter_item_t : aliased string := "agar.gui.widget.fixed_plotter.item_t";
  agar_gui_widget_fixed_plotter_plotter_access_t : aliased string := "agar.gui.widget.fixed_plotter.plotter_access_t";
  agar_gui_widget_fixed_plotter_plotter_t : aliased string := "agar.gui.widget.fixed_plotter.plotter_t";
  agar_gui_widget_fixed_plotter_type_t : aliased string := "agar.gui.widget.fixed_plotter.type_t";
  agar_gui_widget_flag_descr_access_t : aliased string := "agar.gui.widget.flag_descr_access_t";
  agar_gui_widget_flag_descr_t : aliased string := "agar.gui.widget.flag_descr_t";
  agar_gui_widget_graph_edge_access_t : aliased string := "agar.gui.widget.graph.edge_access_t";
  agar_gui_widget_graph_edge_t : aliased string := "agar.gui.widget.graph.edge_t";
  agar_gui_widget_graph_graph_access_t : aliased string := "agar.gui.widget.graph.graph_access_t";
  agar_gui_widget_graph_graph_t : aliased string := "agar.gui.widget.graph.graph_t";
  agar_gui_widget_graph_vertex_access_t : aliased string := "agar.gui.widget.graph.vertex_access_t";
  agar_gui_widget_graph_vertex_style_t : aliased string := "agar.gui.widget.graph.vertex_style_t";
  agar_gui_widget_graph_vertex_t : aliased string := "agar.gui.widget.graph.vertex_t";
  agar_gui_widget_hbox_hbox_access_t : aliased string := "agar.gui.widget.hbox.hbox_access_t";
  agar_gui_widget_hbox_hbox_t : aliased string := "agar.gui.widget.hbox.hbox_t";
  agar_gui_widget_hsvpal_hsvpal_access_t : aliased string := "agar.gui.widget.hsvpal.hsvpal_access_t";
  agar_gui_widget_hsvpal_hsvpal_t : aliased string := "agar.gui.widget.hsvpal.hsvpal_t";
  agar_gui_widget_icon_icon_access_t : aliased string := "agar.gui.widget.icon.icon_access_t";
  agar_gui_widget_icon_icon_t : aliased string := "agar.gui.widget.icon.icon_t";
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
  agar_gui_widget_menu_popup_menu_access_t : aliased string := "agar.gui.widget.menu.popup_menu_access_t";
  agar_gui_widget_menu_popup_menu_t : aliased string := "agar.gui.widget.menu.popup_menu_t";
  agar_gui_widget_menu_view_access_t : aliased string := "agar.gui.widget.menu.view_access_t";
  agar_gui_widget_menu_view_t : aliased string := "agar.gui.widget.menu.view_t";
  agar_gui_widget_mpane_layout_t : aliased string := "agar.gui.widget.mpane.layout_t";
  agar_gui_widget_mpane_mpane_access_t : aliased string := "agar.gui.widget.mpane.mpane_access_t";
  agar_gui_widget_mpane_mpane_t : aliased string := "agar.gui.widget.mpane.mpane_t";
  agar_gui_widget_notebook_notebook_access_t : aliased string := "agar.gui.widget.notebook.notebook_access_t";
  agar_gui_widget_notebook_notebook_t : aliased string := "agar.gui.widget.notebook.notebook_t";
  agar_gui_widget_notebook_tab_access_t : aliased string := "agar.gui.widget.notebook.tab_access_t";
  agar_gui_widget_notebook_tab_alignment_t : aliased string := "agar.gui.widget.notebook.tab_alignment_t";
  agar_gui_widget_notebook_tab_t : aliased string := "agar.gui.widget.notebook.tab_t";
  agar_gui_widget_numerical_numerical_access_t : aliased string := "agar.gui.widget.numerical.numerical_access_t";
  agar_gui_widget_numerical_numerical_t : aliased string := "agar.gui.widget.numerical.numerical_t";
  agar_gui_widget_pane_pane_access_t : aliased string := "agar.gui.widget.pane.pane_access_t";
  agar_gui_widget_pane_pane_t : aliased string := "agar.gui.widget.pane.pane_t";
  agar_gui_widget_pane_type_t : aliased string := "agar.gui.widget.pane.type_t";
  agar_gui_widget_pixmap_pixmap_access_t : aliased string := "agar.gui.widget.pixmap.pixmap_access_t";
  agar_gui_widget_pixmap_pixmap_t : aliased string := "agar.gui.widget.pixmap.pixmap_t";
  agar_gui_widget_progress_bar_progress_bar_access_t : aliased string := "agar.gui.widget.progress_bar.progress_bar_access_t";
  agar_gui_widget_progress_bar_progress_bar_t : aliased string := "agar.gui.widget.progress_bar.progress_bar_t";
  agar_gui_widget_progress_bar_type_t : aliased string := "agar.gui.widget.progress_bar.type_t";
  agar_gui_widget_radio_item_access_t : aliased string := "agar.gui.widget.radio.item_access_t";
  agar_gui_widget_radio_item_t : aliased string := "agar.gui.widget.radio.item_t";
  agar_gui_widget_radio_radio_access_t : aliased string := "agar.gui.widget.radio.radio_access_t";
  agar_gui_widget_radio_radio_t : aliased string := "agar.gui.widget.radio.radio_t";
  agar_gui_widget_scrollbar_button_t : aliased string := "agar.gui.widget.scrollbar.button_t";
  agar_gui_widget_scrollbar_scrollbar_access_t : aliased string := "agar.gui.widget.scrollbar.scrollbar_access_t";
  agar_gui_widget_scrollbar_scrollbar_t : aliased string := "agar.gui.widget.scrollbar.scrollbar_t";
  agar_gui_widget_scrollbar_type_t : aliased string := "agar.gui.widget.scrollbar.type_t";
  agar_gui_widget_separator_separator_access_t : aliased string := "agar.gui.widget.separator.separator_access_t";
  agar_gui_widget_separator_separator_t : aliased string := "agar.gui.widget.separator.separator_t";
  agar_gui_widget_separator_type_t : aliased string := "agar.gui.widget.separator.type_t";
  agar_gui_widget_size_req_access_t : aliased string := "agar.gui.widget.size_req_access_t";
  agar_gui_widget_size_req_t : aliased string := "agar.gui.widget.size_req_t";
  agar_gui_widget_size_spec_t : aliased string := "agar.gui.widget.size_spec_t";
  agar_gui_widget_slider_button_t : aliased string := "agar.gui.widget.slider.button_t";
  agar_gui_widget_slider_slider_access_t : aliased string := "agar.gui.widget.slider.slider_access_t";
  agar_gui_widget_slider_slider_t : aliased string := "agar.gui.widget.slider.slider_t";
  agar_gui_widget_slider_type_t : aliased string := "agar.gui.widget.slider.type_t";
  agar_gui_widget_socket_bg_type_t : aliased string := "agar.gui.widget.socket.bg_type_t";
  agar_gui_widget_socket_socket_access_t : aliased string := "agar.gui.widget.socket.socket_access_t";
  agar_gui_widget_socket_socket_t : aliased string := "agar.gui.widget.socket.socket_t";
  agar_gui_widget_table_cell_access_t : aliased string := "agar.gui.widget.table.cell_access_t";
  agar_gui_widget_table_cell_t : aliased string := "agar.gui.widget.table.cell_t";
  agar_gui_widget_table_cell_type_t : aliased string := "agar.gui.widget.table.cell_type_t";
  agar_gui_widget_table_column_access_t : aliased string := "agar.gui.widget.table.column_access_t";
  agar_gui_widget_table_column_t : aliased string := "agar.gui.widget.table.column_t";
  agar_gui_widget_table_popup_access_t : aliased string := "agar.gui.widget.table.popup_access_t";
  agar_gui_widget_table_popup_t : aliased string := "agar.gui.widget.table.popup_t";
  agar_gui_widget_table_select_mode_t : aliased string := "agar.gui.widget.table.select_mode_t";
  agar_gui_widget_table_table_access_t : aliased string := "agar.gui.widget.table.table_access_t";
  agar_gui_widget_table_table_t : aliased string := "agar.gui.widget.table.table_t";
  agar_gui_widget_textbox_textbox_access_t : aliased string := "agar.gui.widget.textbox.textbox_access_t";
  agar_gui_widget_textbox_textbox_t : aliased string := "agar.gui.widget.textbox.textbox_t";
  agar_gui_widget_titlebar_titlebar_access_t : aliased string := "agar.gui.widget.titlebar.titlebar_access_t";
  agar_gui_widget_titlebar_titlebar_t : aliased string := "agar.gui.widget.titlebar.titlebar_t";
  agar_gui_widget_tlist_item_access_t : aliased string := "agar.gui.widget.tlist.item_access_t";
  agar_gui_widget_tlist_item_t : aliased string := "agar.gui.widget.tlist.item_t";
  agar_gui_widget_tlist_popup_access_t : aliased string := "agar.gui.widget.tlist.popup_access_t";
  agar_gui_widget_tlist_popup_t : aliased string := "agar.gui.widget.tlist.popup_t";
  agar_gui_widget_tlist_tlist_access_t : aliased string := "agar.gui.widget.tlist.tlist_access_t";
  agar_gui_widget_tlist_tlist_t : aliased string := "agar.gui.widget.tlist.tlist_t";
  agar_gui_widget_toolbar_toolbar_access_t : aliased string := "agar.gui.widget.toolbar.toolbar_access_t";
  agar_gui_widget_toolbar_toolbar_t : aliased string := "agar.gui.widget.toolbar.toolbar_t";
  agar_gui_widget_toolbar_type_t : aliased string := "agar.gui.widget.toolbar.type_t";
  agar_gui_widget_ucombo_ucombo_access_t : aliased string := "agar.gui.widget.ucombo.ucombo_access_t";
  agar_gui_widget_ucombo_ucombo_t : aliased string := "agar.gui.widget.ucombo.ucombo_t";
  agar_gui_widget_vbox_vbox_access_t : aliased string := "agar.gui.widget.vbox.vbox_access_t";
  agar_gui_widget_vbox_vbox_t : aliased string := "agar.gui.widget.vbox.vbox_t";
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
    (agar_gui_rect_rect2_access_t'access, agar.gui.rect.rect2_access_t'size),
    (agar_gui_rect_rect2_t'access, agar.gui.rect.rect2_t'size),
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
    (agar_gui_widget_fixed_fixed_access_t'access, gen_agar_gui_widget_fixed.fixed_access_t'size),
    (agar_gui_widget_fixed_fixed_t'access, gen_agar_gui_widget_fixed.fixed_t'size),
    (agar_gui_widget_fixed_plotter_item_access_t'access, agar.gui.widget.fixed_plotter.item_access_t'size),
    (agar_gui_widget_fixed_plotter_item_t'access, agar.gui.widget.fixed_plotter.item_t'size),
    (agar_gui_widget_fixed_plotter_plotter_access_t'access, agar.gui.widget.fixed_plotter.plotter_access_t'size),
    (agar_gui_widget_fixed_plotter_plotter_t'access, agar.gui.widget.fixed_plotter.plotter_t'size),
    (agar_gui_widget_fixed_plotter_type_t'access, agar.gui.widget.fixed_plotter.type_t'size),
    (agar_gui_widget_flag_descr_access_t'access, agar.gui.widget.flag_descr_access_t'size),
    (agar_gui_widget_flag_descr_t'access, agar.gui.widget.flag_descr_t'size),
    (agar_gui_widget_graph_edge_access_t'access, agar.gui.widget.graph.edge_access_t'size),
    (agar_gui_widget_graph_edge_t'access, agar.gui.widget.graph.edge_t'size),
    (agar_gui_widget_graph_graph_access_t'access, agar.gui.widget.graph.graph_access_t'size),
    (agar_gui_widget_graph_graph_t'access, agar.gui.widget.graph.graph_t'size),
    (agar_gui_widget_graph_vertex_access_t'access, agar.gui.widget.graph.vertex_access_t'size),
    (agar_gui_widget_graph_vertex_style_t'access, agar.gui.widget.graph.vertex_style_t'size),
    (agar_gui_widget_graph_vertex_t'access, agar.gui.widget.graph.vertex_t'size),
    (agar_gui_widget_hbox_hbox_access_t'access, agar.gui.widget.hbox.hbox_access_t'size),
    (agar_gui_widget_hbox_hbox_t'access, agar.gui.widget.hbox.hbox_t'size),
    (agar_gui_widget_hsvpal_hsvpal_access_t'access, agar.gui.widget.hsvpal.hsvpal_access_t'size),
    (agar_gui_widget_hsvpal_hsvpal_t'access, agar.gui.widget.hsvpal.hsvpal_t'size),
    (agar_gui_widget_icon_icon_access_t'access, agar.gui.widget.icon.icon_access_t'size),
    (agar_gui_widget_icon_icon_t'access, agar.gui.widget.icon.icon_t'size),
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
    (agar_gui_widget_menu_popup_menu_access_t'access, agar.gui.widget.menu.popup_menu_access_t'size),
    (agar_gui_widget_menu_popup_menu_t'access, agar.gui.widget.menu.popup_menu_t'size),
    (agar_gui_widget_menu_view_access_t'access, agar.gui.widget.menu.view_access_t'size),
    (agar_gui_widget_menu_view_t'access, agar.gui.widget.menu.view_t'size),
    (agar_gui_widget_mpane_layout_t'access, agar.gui.widget.mpane.layout_t'size),
    (agar_gui_widget_mpane_mpane_access_t'access, agar.gui.widget.mpane.mpane_access_t'size),
    (agar_gui_widget_mpane_mpane_t'access, agar.gui.widget.mpane.mpane_t'size),
    (agar_gui_widget_notebook_notebook_access_t'access, agar.gui.widget.notebook.notebook_access_t'size),
    (agar_gui_widget_notebook_notebook_t'access, agar.gui.widget.notebook.notebook_t'size),
    (agar_gui_widget_notebook_tab_access_t'access, agar.gui.widget.notebook.tab_access_t'size),
    (agar_gui_widget_notebook_tab_alignment_t'access, agar.gui.widget.notebook.tab_alignment_t'size),
    (agar_gui_widget_notebook_tab_t'access, agar.gui.widget.notebook.tab_t'size),
    (agar_gui_widget_numerical_numerical_access_t'access, agar.gui.widget.numerical.numerical_access_t'size),
    (agar_gui_widget_numerical_numerical_t'access, agar.gui.widget.numerical.numerical_t'size),
    (agar_gui_widget_pane_pane_access_t'access, agar.gui.widget.pane.pane_access_t'size),
    (agar_gui_widget_pane_pane_t'access, agar.gui.widget.pane.pane_t'size),
    (agar_gui_widget_pane_type_t'access, agar.gui.widget.pane.type_t'size),
    (agar_gui_widget_pixmap_pixmap_access_t'access, agar.gui.widget.pixmap.pixmap_access_t'size),
    (agar_gui_widget_pixmap_pixmap_t'access, agar.gui.widget.pixmap.pixmap_t'size),
    (agar_gui_widget_progress_bar_progress_bar_access_t'access, agar.gui.widget.progress_bar.progress_bar_access_t'size),
    (agar_gui_widget_progress_bar_progress_bar_t'access, agar.gui.widget.progress_bar.progress_bar_t'size),
    (agar_gui_widget_progress_bar_type_t'access, agar.gui.widget.progress_bar.type_t'size),
    (agar_gui_widget_radio_item_access_t'access, agar.gui.widget.radio.item_access_t'size),
    (agar_gui_widget_radio_item_t'access, agar.gui.widget.radio.item_t'size),
    (agar_gui_widget_radio_radio_access_t'access, agar.gui.widget.radio.radio_access_t'size),
    (agar_gui_widget_radio_radio_t'access, agar.gui.widget.radio.radio_t'size),
    (agar_gui_widget_scrollbar_button_t'access, agar.gui.widget.scrollbar.button_t'size),
    (agar_gui_widget_scrollbar_scrollbar_access_t'access, agar.gui.widget.scrollbar.scrollbar_access_t'size),
    (agar_gui_widget_scrollbar_scrollbar_t'access, agar.gui.widget.scrollbar.scrollbar_t'size),
    (agar_gui_widget_scrollbar_type_t'access, agar.gui.widget.scrollbar.type_t'size),
    (agar_gui_widget_separator_separator_access_t'access, agar.gui.widget.separator.separator_access_t'size),
    (agar_gui_widget_separator_separator_t'access, agar.gui.widget.separator.separator_t'size),
    (agar_gui_widget_separator_type_t'access, agar.gui.widget.separator.type_t'size),
    (agar_gui_widget_size_req_access_t'access, agar.gui.widget.size_req_access_t'size),
    (agar_gui_widget_size_req_t'access, agar.gui.widget.size_req_t'size),
    (agar_gui_widget_size_spec_t'access, agar.gui.widget.size_spec_t'size),
    (agar_gui_widget_slider_button_t'access, agar.gui.widget.slider.button_t'size),
    (agar_gui_widget_slider_slider_access_t'access, agar.gui.widget.slider.slider_access_t'size),
    (agar_gui_widget_slider_slider_t'access, agar.gui.widget.slider.slider_t'size),
    (agar_gui_widget_slider_type_t'access, agar.gui.widget.slider.type_t'size),
    (agar_gui_widget_socket_bg_type_t'access, agar.gui.widget.socket.bg_type_t'size),
    (agar_gui_widget_socket_socket_access_t'access, agar.gui.widget.socket.socket_access_t'size),
    (agar_gui_widget_socket_socket_t'access, agar.gui.widget.socket.socket_t'size),
    (agar_gui_widget_table_cell_access_t'access, agar.gui.widget.table.cell_access_t'size),
    (agar_gui_widget_table_cell_t'access, agar.gui.widget.table.cell_t'size),
    (agar_gui_widget_table_cell_type_t'access, agar.gui.widget.table.cell_type_t'size),
    (agar_gui_widget_table_column_access_t'access, agar.gui.widget.table.column_access_t'size),
    (agar_gui_widget_table_column_t'access, agar.gui.widget.table.column_t'size),
    (agar_gui_widget_table_popup_access_t'access, agar.gui.widget.table.popup_access_t'size),
    (agar_gui_widget_table_popup_t'access, agar.gui.widget.table.popup_t'size),
    (agar_gui_widget_table_select_mode_t'access, agar.gui.widget.table.select_mode_t'size),
    (agar_gui_widget_table_table_access_t'access, agar.gui.widget.table.table_access_t'size),
    (agar_gui_widget_table_table_t'access, agar.gui.widget.table.table_t'size),
    (agar_gui_widget_textbox_textbox_access_t'access, agar.gui.widget.textbox.textbox_access_t'size),
    (agar_gui_widget_textbox_textbox_t'access, agar.gui.widget.textbox.textbox_t'size),
    (agar_gui_widget_titlebar_titlebar_access_t'access, agar.gui.widget.titlebar.titlebar_access_t'size),
    (agar_gui_widget_titlebar_titlebar_t'access, agar.gui.widget.titlebar.titlebar_t'size),
    (agar_gui_widget_tlist_item_access_t'access, agar.gui.widget.tlist.item_access_t'size),
    (agar_gui_widget_tlist_item_t'access, agar.gui.widget.tlist.item_t'size),
    (agar_gui_widget_tlist_popup_access_t'access, agar.gui.widget.tlist.popup_access_t'size),
    (agar_gui_widget_tlist_popup_t'access, agar.gui.widget.tlist.popup_t'size),
    (agar_gui_widget_tlist_tlist_access_t'access, agar.gui.widget.tlist.tlist_access_t'size),
    (agar_gui_widget_tlist_tlist_t'access, agar.gui.widget.tlist.tlist_t'size),
    (agar_gui_widget_toolbar_toolbar_access_t'access, agar.gui.widget.toolbar.toolbar_access_t'size),
    (agar_gui_widget_toolbar_toolbar_t'access, agar.gui.widget.toolbar.toolbar_t'size),
    (agar_gui_widget_toolbar_type_t'access, agar.gui.widget.toolbar.type_t'size),
    (agar_gui_widget_ucombo_ucombo_access_t'access, agar.gui.widget.ucombo.ucombo_access_t'size),
    (agar_gui_widget_ucombo_ucombo_t'access, agar.gui.widget.ucombo.ucombo_t'size),
    (agar_gui_widget_vbox_vbox_access_t'access, agar.gui.widget.vbox.vbox_access_t'size),
    (agar_gui_widget_vbox_vbox_t'access, agar.gui.widget.vbox.vbox_t'size),
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
