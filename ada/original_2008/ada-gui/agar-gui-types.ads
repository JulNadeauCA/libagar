-- This file only exists because of circular type dependencies in the C
-- library. Types must therefore be collected here, imported and renamed
-- in each package to prevent loops.

with agar.core.tail_queue;
with agar.core.types;
with agar.gui.rect;
with agar.gui.widget.box;
with agar.gui.widget.button;
with agar.gui.widget.label;
with agar.gui.widget;

package agar.gui.types is

  -- widget.icon

  type widget_icon_t is limited private;
  type widget_icon_access_t is access all widget_icon_t;
  pragma convention (c, widget_icon_access_t);

  type widget_icon_flags_t is new c.unsigned;
  pragma convention (c, widget_icon_flags_t);

  function widget_icon_widget (icon : widget_icon_access_t) return agar.gui.widget.widget_access_t;
  pragma inline (widget_icon_widget);

  -- window

  type window_t is limited private;
  type window_access_t is access all window_t;
  pragma convention (c, window_access_t);

  type window_flags_t is new c.unsigned;
  pragma convention (c, window_flags_t);

  package window_tail_queue is new agar.core.tail_queue
    (entry_type => window_access_t);

  type window_alignment_t is (
    WINDOW_TL,
    WINDOW_TC,
    WINDOW_TR,
    WINDOW_ML,
    WINDOW_MC,
    WINDOW_MR,
    WINDOW_BL,
    WINDOW_BC,
    WINDOW_BR
  );
  for window_alignment_t use (
    WINDOW_TL => 0,
    WINDOW_TC => 1,
    WINDOW_TR => 2,
    WINDOW_ML => 3,
    WINDOW_MC => 4,
    WINDOW_MR => 5,
    WINDOW_BL => 6,
    WINDOW_BC => 7,
    WINDOW_BR => 8
  );
  for window_alignment_t'size use c.unsigned'size;
  pragma convention (c, window_alignment_t);

  window_caption_max : constant c.unsigned := 512;

  function window_widget (window : window_access_t) return agar.gui.widget.widget_access_t;
  pragma inline (window_widget);

  -- widget.titlebar

  type widget_titlebar_t is limited private;
  type widget_titlebar_access_t is access all widget_titlebar_t;
  pragma convention (c, widget_titlebar_access_t);

  type widget_titlebar_flags_t is new c.unsigned;

  function widget_titlebar_widget (titlebar : widget_titlebar_access_t)
    return agar.gui.widget.widget_access_t;
  pragma inline (widget_titlebar_widget);

private

  type widget_icon_name_t is array (1 .. agar.gui.widget.label.max) of aliased c.char;
  pragma convention (c, widget_icon_name_t);

  type widget_icon_t is record
    widget        : aliased agar.gui.widget.widget_t;
    flags         : widget_icon_flags_t;
    surface       : c.int;
    label_text    : widget_icon_name_t;
    label_surface : c.int;
    label_pad     : c.int;
    window        : window_access_t;
    socket        : agar.core.types.void_ptr_t;
    x_saved       : c.int;
    y_saved       : c.int;
    w_saved       : c.int;
    h_saved       : c.int;
    c_background  : agar.core.types.uint32_t;
  end record;
  pragma convention (c, widget_icon_t);

  type window_caption_t is array (1 .. window_caption_max) of aliased c.char;
  pragma convention (c, window_caption_t);

  type window_t is record
    widget      : aliased agar.gui.widget.widget_t;
    flags       : window_flags_t;

    caption     : window_caption_t;
    visible     : c.int;

    tbar        : widget_titlebar_access_t;
    alignment   : window_alignment_t;
    spacing     : c.int;
    tpad        : c.int;
    bpad        : c.int;
    lpad        : c.int;
    rpad        : c.int;

    reqw        : c.int;
    reqh        : c.int;

    minw        : c.int;
    minh        : c.int;

    border_bot  : c.int;
    border_side : c.int;
    resize_ctrl : c.int;

    r_saved     : agar.gui.rect.rect_t;
    min_pct     : c.int;

    subwins     : window_tail_queue.head_t;
    windows     : window_tail_queue.entry_t;
    swins       : window_tail_queue.entry_t;
    detach      : window_tail_queue.entry_t;

    icon        : widget_icon_access_t;
    r           : agar.gui.rect.rect_t;
  end record;
  pragma convention (c, window_t);

  type widget_titlebar_t is record
    box             : aliased agar.gui.widget.box.box_t;
    flags           : widget_titlebar_flags_t;
    pressed         : c.int;
    window          : window_access_t;
    label           : agar.gui.widget.label.label_access_t;
    close_button    : agar.gui.widget.button.button_access_t;
    minimize_button : agar.gui.widget.button.button_access_t;
    maximize_button : agar.gui.widget.button.button_access_t;
  end record;
  pragma convention (c, widget_titlebar_t);

end agar.gui.types;
