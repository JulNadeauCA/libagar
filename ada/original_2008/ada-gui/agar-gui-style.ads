with agar.core.types;
with agar.gui.rect;
with agar.gui.widget;
with agar.gui.window;
with interfaces.c.strings;

package agar.gui.style is
  package cs renames interfaces.c.strings;

  type version_t is record
    major : c.int;
    minor : c.int;
  end record;
  type version_access_t is access all version_t;
  pragma convention (c, version_t);
  pragma convention (c, version_access_t);

  type style_t;
  type style_access_t is access all style_t;

  type style_t is record
    name                               : cs.chars_ptr;
    version                            : version_t;
    init                               : access procedure (style : style_access_t);
    destroy                            : access procedure (style : style_access_t);
    window                             : access procedure (window : agar.gui.window.window_access_t);
    titlebar_background                : access procedure
      (widget     : agar.gui.widget.widget_access_t;
       is_pressed : c.int;
       is_focused : c.int);
    button_background                  : access procedure
      (widget     : agar.gui.widget.widget_access_t;
       is_pressed : c.int);
    button_text_offset                 : access procedure
      (widget     : agar.gui.widget.widget_access_t;
       is_pressed : c.int;
       x          : access c.int;
       y          : access c.int);
    box_frame                          : access procedure
      (widget : agar.gui.widget.widget_access_t;
       depth  : c.int);
    checkbox_button                    : access procedure
      (widget : agar.gui.widget.widget_access_t;
       state  : c.int);
    console_background                 : access procedure
      (widget : agar.gui.widget.widget_access_t;
       bg     : agar.core.types.uint32_t);
    fixed_plotter_background           : access procedure
      (widget    : agar.gui.widget.widget_access_t;
       show_axis : c.int;
       y_offset  : agar.core.types.uint32_t);
    menu_root_background               : access procedure
      (widget : agar.gui.widget.widget_access_t);
    menu_root_selected_item_background : access procedure
      (widget : agar.gui.widget.widget_access_t;
       rect   : agar.gui.rect.rect_t);
    menu_background                    : access procedure
      (widget : agar.gui.widget.widget_access_t;
       rect   : agar.gui.rect.rect_t);
    menu_item_background               : access procedure
      (widget      : agar.gui.widget.widget_access_t;
       rect        : agar.gui.rect.rect_t;
       x_icon      : c.int;
       icon_obj    : agar.gui.widget.widget_access_t;
       icon        : c.int;
       is_selected : c.int;
       bool_state  : c.int);
    menu_item_separator                : access procedure
      (widget : agar.gui.widget.widget_access_t;
       x1     : c.int;
       x2     : c.int;
       y      : c.int;
       h      : c.int);
    notebook_background                : access procedure
      (widget   : agar.gui.widget.widget_access_t;
       rect     : agar.gui.rect.rect_t);
    notebook_tab_background            : access procedure
      (widget      : agar.gui.widget.widget_access_t;
       rect        : agar.gui.rect.rect_t;
       index       : c.int;
       is_selected : c.int);
    pane_horizontal_divider            : access procedure
      (widget    : agar.gui.widget.widget_access_t;
       x         : c.int;
       y         : c.int;
       w         : c.int;
       h         : c.int;
       is_moving : c.int);
    pane_vertical_divider              : access procedure
      (widget    : agar.gui.widget.widget_access_t;
       x         : c.int;
       y         : c.int;
       w         : c.int;
       h         : c.int;
       is_moving : c.int);
    radio_group_background             : access procedure
      (widget : agar.gui.widget.widget_access_t;
       rect   : agar.gui.rect.rect_t);
    radio_button                       : access procedure
      (widget   : agar.gui.widget.widget_access_t; -- XXX: AG_Radio *
       x        : c.int;
       y        : c.int;
       selected : c.int;
       over     : c.int);
    progress_bar_background            : access procedure
      (widget : agar.gui.widget.widget_access_t);
    scrollbar_vertical                 : access procedure
      (widget : agar.gui.widget.widget_access_t;  -- XXX: AG_Scrollbar *
       y      : c.int;
       h      : c.int);
    scrollbar_horizontal               : access procedure
      (widget : agar.gui.widget.widget_access_t;  -- XXX: AG_Scrollbar *
       x      : c.int;
       w      : c.int);
    slider_background_horizontal       : access procedure
      (widget : agar.gui.widget.widget_access_t);
    slider_background_vertical         : access procedure
      (widget : agar.gui.widget.widget_access_t);
    slider_control_horizontal          : access procedure
      (widget : agar.gui.widget.widget_access_t;
       x      : c.int;
       y      : c.int);
    slider_control_vertical            : access procedure
      (widget : agar.gui.widget.widget_access_t;
       y      : c.int;
       h      : c.int);
    separator_horizontal               : access procedure
      (widget : agar.gui.widget.widget_access_t); -- XXX: AG_Separator *
    separator_vertical                 : access procedure
      (widget : agar.gui.widget.widget_access_t); -- XXX: AG_Separator *
    socket_background                  : access procedure
      (widget : agar.gui.widget.widget_access_t); -- XXX: AG_Socket *
    socket_overlay                     : access procedure
      (widget    : agar.gui.widget.widget_access_t; -- XXX: AG_Socket *
       highlight : c.int);
    table_background                   : access procedure
      (widget : agar.gui.widget.widget_access_t;
       rect   : agar.gui.rect.rect_t);
    table_column_header_background     : access procedure
      (widget      : agar.gui.widget.widget_access_t;
       index       : c.int;
       rect        : agar.gui.rect.rect_t;
       is_selected : c.int);
    table_selected_column_background   : access procedure
      (widget : agar.gui.widget.widget_access_t;
       index  : c.int;
       rect   : agar.gui.rect.rect_t);
    table_row_background               : access procedure
      (widget      : agar.gui.widget.widget_access_t;
       rect        : agar.gui.rect.rect_t;
       is_selected : c.int);
    textbox_background                 : access procedure
      (widget   : agar.gui.widget.widget_access_t;
       rect     : agar.gui.rect.rect_t;
       is_combo : c.int);
    list_background                    : access procedure
      (widget : agar.gui.widget.widget_access_t;
       rect   : agar.gui.rect.rect_t);
    list_item_background               : access procedure
      (widget      : agar.gui.widget.widget_access_t;
       rect        : agar.gui.rect.rect_t;
       is_selected : c.int);
    tree_subnode_indicator             : access procedure
      (widget      : agar.gui.widget.widget_access_t;
       rect        : agar.gui.rect.rect_t;
       is_expanded : c.int);
  end record;
  pragma convention (c, style_t);
  pragma convention (c, style_access_t);

  procedure set_style
    (widget : agar.gui.widget.widget_access_t;
     style  : style_access_t);
  procedure set_style
    (widget : agar.gui.window.window_access_t;
     style  : style_access_t);
  pragma import (c, set_style, "AG_SetStyle");

end agar.gui.style;
