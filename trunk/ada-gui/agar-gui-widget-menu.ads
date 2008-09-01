with SDL;
with SDL.keysym;
with agar.gui.widget.button;
with agar.gui.widget.toolbar;
with agar.gui.window;
with agar.core.event;
with agar.core.slist;
with agar.core.timeout;

package agar.gui.widget.menu is

  use type c.unsigned;

  -- forward declarations

  type item_t;
  type item_access_t is access all item_t;
  pragma convention (c, item_access_t);

  type menu_t;
  type menu_access_t is access all menu_t;
  pragma convention (c, menu_access_t);

  type popup_menu_t;
  type popup_menu_access_t is access all popup_menu_t;
  pragma convention (c, popup_menu_access_t);

  type view_t;
  type view_access_t is access all view_t;
  pragma convention (c, view_access_t);

  package menu_slist is new agar.core.slist
    (entry_type => menu_access_t);

  -- types

  type binding_t is (
    MENU_NO_BINDING,
    MENU_INT_BOOL,
    MENU_INT8_BOOL,
    MENU_INT_FLAGS,
    MENU_INT8_FLAGS,
    MENU_INT16_FLAGS,
    MENU_INT32_FLAGS
  );
  for binding_t use (
    MENU_NO_BINDING  => 0,
    MENU_INT_BOOL    => 1,
    MENU_INT8_BOOL   => 2,
    MENU_INT_FLAGS   => 3,
    MENU_INT8_FLAGS  => 4,
    MENU_INT16_FLAGS => 5,
    MENU_INT32_FLAGS => 6
  );
  for binding_t'size use c.unsigned'size;
  pragma convention (c, binding_t);

  subtype item_flags_t is c.unsigned;
  ITEM_ICONS     : constant item_flags_t := 16#01#;
  ITEM_NOSELECT  : constant item_flags_t := 16#02#;
  ITEM_SEPARATOR : constant item_flags_t := 16#04#;

  type item_t is record
    text             : cs.chars_ptr;
    label_enabled    : c.int;
    label_disabled   : c.int;
    icon             : c.int;
    icon_source      : agar.gui.surface.surface_access_t;
    value            : c.int;
    state            : c.int;
    key_equiv        : SDL.keysym.key_t;
    key_mod          : SDL.keysym.modkey_t;
    x                : c.int;
    y                : c.int;
    subitems         : item_access_t;
    nsubitems        : c.unsigned;
    click_func       : access agar.core.event.event_t;
    poll             : access agar.core.event.event_t;
    flags            : item_flags_t;
    bind_type        : binding_t;
    bind_ptr         : agar.core.types.void_ptr_t;
    bind_flags       : agar.core.types.uint32_t;
    bind_invert      : c.int;
    bind_lock        : agar.core.threads.mutex_t;
    view             : view_access_t;
    parent           : menu_access_t;
    parent_item      : item_access_t;
    selected_subitem : item_access_t;
    toolbar_button   : agar.gui.widget.button.button_access_t;
  end record;
  pragma convention (c, item_t);

  subtype menu_flags_t is c.unsigned;
  MENU_HFILL  : constant menu_flags_t := 16#01#;
  MENU_VFILL  : constant menu_flags_t := 16#02#;
  MENU_EXPAND : constant menu_flags_t := MENU_HFILL or MENU_VFILL;
  MENU_GLOBAL : constant menu_flags_t := 16#04#;

  type menu_t is record
    widget             : widget_t;
    flags              : menu_flags_t;
    root               : item_access_t;
    selecting          : c.int;
    item_selected      : item_access_t;
    spacing_horizontal : c.int;
    spacing_vertical   : c.int;
    pad_left           : c.int;
    pad_right          : c.int;
    pad_top            : c.int;
    pad_bottom         : c.int;
    label_pad_left     : c.int;
    label_pad_right    : c.int;
    label_pad_top      : c.int;
    label_pad_bottom   : c.int;
    height             : c.int;
    current_state      : c.int;
    current_toolbar    : agar.gui.widget.toolbar.toolbar_access_t;
  end record;
  pragma convention (c, menu_t);

  type popup_menu_t is record
    menu   : menu_access_t;
    item   : item_access_t;
    window : agar.gui.window.window_access_t;
    menus  : menu_slist.entry_t;
  end record;
  pragma convention (c, popup_menu_t);

  type view_t is record
    widget              : widget_t;
    panel               : agar.gui.window.window_access_t;
    panel_menu          : menu_access_t;
    panel_item          : item_access_t;
    icon_label_spacing  : c.int;
    label_arrow_spacing : c.int;
    pad_left            : c.int;
    pad_right           : c.int;
    pad_top             : c.int;
    pad_bottom          : c.int;
    submenu_to          : agar.core.timeout.timeout_t;
  end record;
  pragma convention (c, view_t);

  --

end agar.gui.widget.menu;
