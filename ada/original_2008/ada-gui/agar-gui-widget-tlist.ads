with agar.core.event;
with agar.core.event_types;
with agar.core.tail_queue;
with agar.core.timeout;
with agar.core.types;
with agar.gui.surface;
with agar.gui.widget.label;
with agar.gui.widget.menu;
with agar.gui.widget.scrollbar;
with agar.gui.window;

package agar.gui.widget.tlist is

  use type c.unsigned;

  type popup_t is limited private;
  type popup_access_t is access all popup_t;
  pragma convention (c, popup_access_t);

  type item_t is limited private;
  type item_access_t is access all item_t;
  pragma convention (c, item_access_t);

  type tlist_t is limited private;
  type tlist_access_t is access all tlist_t;
  pragma convention (c, tlist_access_t);

  package popup_tail_queue is new agar.core.tail_queue
    (entry_type => popup_access_t);
  package item_tail_queue is new agar.core.tail_queue
    (entry_type => item_access_t);
  package tlist_tail_queue is new agar.core.tail_queue
    (entry_type => tlist_access_t);

  type flags_t is new c.unsigned;
  TLIST_MULTI       : constant flags_t := 16#001#;
  TLIST_MULTITOGGLE : constant flags_t := 16#002#;
  TLIST_POLL        : constant flags_t := 16#004#;
  TLIST_TREE        : constant flags_t := 16#010#;
  TLIST_HFILL       : constant flags_t := 16#020#;
  TLIST_VFILL       : constant flags_t := 16#040#;
  TLIST_NOSELSTATE  : constant flags_t := 16#100#;
  TLIST_EXPAND      : constant flags_t := TLIST_HFILL or TLIST_VFILL;

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return tlist_access_t;
  pragma import (c, allocate, "AG_TlistNew");

  function allocate_polled
    (parent   : widget_access_t;
     flags    : flags_t;
     callback : agar.core.event.callback_t) return tlist_access_t;
  pragma import (c, allocate_polled, "AG_TlistNewPolled");

  procedure set_item_height
    (tlist  : tlist_access_t;
     height : natural);
  pragma inline (set_item_height);

  procedure set_icon
    (tlist : tlist_access_t;
     item  : item_access_t;
     icon  : agar.gui.surface.surface_access_t);
  pragma import (c, set_icon, "AG_TlistSetIcon");

  procedure size_hint
    (tlist     : tlist_access_t;
     text      : string;
     num_items : natural);
  pragma inline (size_hint);

  procedure size_hint_pixels
    (tlist     : tlist_access_t;
     width     : natural;
     num_items : natural);
  pragma inline (size_hint_pixels);

  procedure size_hint_largest
    (tlist     : tlist_access_t;
     num_items : natural);
  pragma inline (size_hint_largest);

  procedure set_double_click_callback
    (tlist     : tlist_access_t;
     callback  : agar.core.event.callback_t);
  pragma inline (set_double_click_callback);

  procedure set_changed_callback
    (tlist     : tlist_access_t;
     callback  : agar.core.event.callback_t);
  pragma inline (set_changed_callback);

  -- manipulating items

  procedure delete
    (tlist : tlist_access_t;
     item  : item_access_t);
  pragma import (c, delete, "AG_TlistDel");

  procedure list_begin (tlist : tlist_access_t);
  pragma import (c, list_begin, "AG_TlistClear");

  procedure list_end (tlist : tlist_access_t);
  pragma import (c, list_end, "AG_TlistRestore");

  procedure list_select
    (tlist : tlist_access_t;
     item  : item_access_t);
  pragma import (c, list_select, "AG_TlistSelect");

  procedure list_select_all (tlist : tlist_access_t);
  pragma import (c, list_select_all, "AG_TlistSelectAll");

  procedure list_deselect
    (tlist : tlist_access_t;
     item  : item_access_t);
  pragma import (c, list_deselect, "AG_TlistDeselect");

  procedure list_deselect_all (tlist : tlist_access_t);
  pragma import (c, list_deselect_all, "AG_TlistDeselectAll");

  function list_select_pointer
    (tlist   : tlist_access_t;
     pointer : agar.core.types.void_ptr_t) return item_access_t;
  pragma import (c, list_select_pointer, "AG_TlistSelectPtr");

  function list_select_text
    (tlist : tlist_access_t;
     text  : string) return item_access_t;
  pragma inline (list_select_text);

  function list_find_by_index
    (tlist : tlist_access_t;
     index : integer) return item_access_t;
  pragma inline (list_find_by_index);

  function list_selected_item (tlist : tlist_access_t) return item_access_t;
  pragma import (c, list_selected_item, "AG_TlistSelectedItem");

  function list_selected_item_pointer (tlist : tlist_access_t) return agar.core.types.void_ptr_t;
  pragma import (c, list_selected_item_pointer, "AG_TlistSelectedItemPtr");

  function list_first (tlist : tlist_access_t) return item_access_t;
  pragma import (c, list_first, "AG_TlistFirstItem");

  function list_last (tlist : tlist_access_t) return item_access_t;
  pragma import (c, list_last, "AG_TlistLastItem");

  -- popup menus

  function set_popup_callback
    (tlist    : tlist_access_t;
     callback : agar.core.event.callback_t) return agar.gui.widget.menu.item_access_t;
  pragma inline (set_popup_callback);

  function set_popup
    (tlist    : tlist_access_t;
     category : string) return agar.gui.widget.menu.item_access_t;
  pragma inline (set_popup);

  --

  function widget (tlist : tlist_access_t) return widget_access_t;
  pragma inline (widget);

private

  type popup_t is record
    class  : cs.chars_ptr;
    menu   : agar.gui.widget.menu.menu_access_t;
    item   : agar.gui.widget.menu.item_access_t;
    panel  : agar.gui.window.window_access_t;
    popups : popup_tail_queue.entry_t;
  end record;
  pragma convention (c, popup_t);

  type item_label_t is array (1 .. agar.gui.widget.label.max) of aliased c.char;
  pragma convention (c, item_label_t);

  type item_argv_t is array (1 .. 8) of aliased agar.core.event_types.arg_t;
  pragma convention (c, item_argv_t);

  type item_t is record
    selected    : c.int;
    icon_source : agar.gui.surface.surface_access_t;
    icon        : c.int;
    ptr         : agar.core.types.void_ptr_t;
    category    : cs.chars_ptr;
    text        : item_label_t;
    label       : c.int;
    argv        : item_argv_t;
    argc        : c.int;
    depth       : agar.core.types.uint8_t;
    flags       : agar.core.types.uint8_t;
    items       : item_tail_queue.entry_t;
    selitems    : item_tail_queue.entry_t;
  end record;
  pragma convention (c, item_t);

  type tlist_t is record
    widget          : aliased widget_t;
    flags           : flags_t;

    selected        : agar.core.types.void_ptr_t;
    hint_width      : c.int;
    hint_height     : c.int;
    space           : c.int;

    item_height     : c.int;
    icon_width      : c.int;
    double_clicked  : agar.core.types.void_ptr_t;
    items           : item_tail_queue.head_t;
    selitems        : item_tail_queue.head_t;
    num_items       : c.int;
    visible_items   : c.int;
    sbar            : agar.gui.widget.scrollbar.scrollbar_access_t;
    popups          : item_tail_queue.head_t;
    compare_func    : access function (a, b : item_access_t) return c.int;
    popup_ev        : access agar.core.event.event_t;
    changed_ev      : access agar.core.event.event_t;
    double_click_ev : access agar.core.event.event_t;
    inc_to          : agar.core.timeout.timeout_t;
    dec_to          : agar.core.timeout.timeout_t;
    wheel_ticks     : agar.core.types.uint32_t;
    row_width       : c.int;
    r               : agar.gui.rect.rect_t;
  end record;
  pragma convention (c, tlist_t);

end agar.gui.widget.tlist;
