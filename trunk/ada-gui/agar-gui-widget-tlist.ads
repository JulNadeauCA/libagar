with agar.core.event;
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

  type popup_t;
  type popup_access_t is access all popup_t;
  pragma convention (c, popup_access_t);

  type item_t;
  type item_access_t is access all item_t;
  pragma convention (c, item_access_t);

  type tlist_t;
  type tlist_access_t is access all tlist_t;
  pragma convention (c, tlist_access_t);

  package popup_tail_queue is new agar.core.tail_queue
    (entry_type => popup_access_t);
  package item_tail_queue is new agar.core.tail_queue
    (entry_type => item_access_t);
  package tlist_tail_queue is new agar.core.tail_queue
    (entry_type => tlist_access_t);

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

  type item_argv_t is array (1 .. 8) of aliased agar.core.event.arg_t;
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

  subtype tlist_flags_t is c.unsigned;
  TLIST_MULTI       : constant tlist_flags_t := 16#001#;
  TLIST_MULTITOGGLE : constant tlist_flags_t := 16#002#;
  TLIST_POLL        : constant tlist_flags_t := 16#004#;
  TLIST_TREE        : constant tlist_flags_t := 16#010#;
  TLIST_HFILL       : constant tlist_flags_t := 16#020#;
  TLIST_VFILL       : constant tlist_flags_t := 16#040#;
  TLIST_NOSELSTATE  : constant tlist_flags_t := 16#100#;
  TLIST_EXPAND      : constant tlist_flags_t := TLIST_HFILL or TLIST_VFILL;

  type tlist_t is record
    widget          : widget_t;
    flags           : tlist_flags_t;
    selected        : agar.core.types.void_ptr_t;
    hint_width      : c.int;
    hint_height     : c.int;
    space           : c.int;
    item_height     : c.int;
    item_width      : c.int;
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
  end record;
  pragma convention (c, tlist_t);

end agar.gui.widget.tlist;
