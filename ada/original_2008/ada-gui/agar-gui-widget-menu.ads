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

  subtype mask_t is c.unsigned;
  subtype mask8_t is agar.core.types.uint8_t;
  subtype mask16_t is agar.core.types.uint16_t;
  subtype mask32_t is agar.core.types.uint32_t;

  type state_t is (FROM_BINDING, DISABLED, ENABLED);
  for state_t use
    (FROM_BINDING => -1,
     DISABLED     => 0,
     ENABLED      => 1);
  for state_t'size use c.unsigned'size;
  pragma convention (c, state_t);

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
    key_equiv        : c.int;
    key_mod          : c.int;
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
    widget             : aliased widget_t;
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
    r                  : agar.gui.rect.rect_t;
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
    widget              : aliased widget_t;
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

  function allocate
    (widget : widget_access_t;
     flags  : flags_t) return menu_access_t;
  pragma import (c, allocate, "AG_MenuNew");

  function allocate_global (flags : flags_t) return menu_access_t;
  pragma import (c, allocate_global, "AG_MenuNewGlobal");

  procedure expand
    (menu : menu_access_t;
     item : item_access_t;
     x    : natural;
     y    : natural);
  pragma inline (expand);

  procedure collapse
    (menu : menu_access_t;
     item : item_access_t);
  pragma import (c, collapse, "AG_MenuCollapse");

  procedure set_padding
    (menu   : menu_access_t;
     left   : natural;
     right  : natural;
     top    : natural;
     bottom : natural);
  pragma inline (set_padding);

  procedure set_label_padding
    (menu   : menu_access_t;
     left   : natural;
     right  : natural;
     top    : natural;
     bottom : natural);
  pragma inline (set_label_padding);

  -- menu items

  function node
    (parent : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t) return item_access_t;
  pragma inline (node);

  function action
    (parent : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     func   : agar.core.event.callback_t) return item_access_t;
  pragma inline (action);

  function action_keyboard
    (parent : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     key    : c.int;
     modkey : c.int;
     func   : agar.core.event.callback_t) return item_access_t;
  pragma inline (action_keyboard);

  function dynamic_item
    (parent : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     func   : agar.core.event.callback_t) return item_access_t;
  pragma inline (dynamic_item);

  function dynamic_item_keyboard
    (parent : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     key    : c.int;
     modkey : c.int;
     func   : agar.core.event.callback_t) return item_access_t;
  pragma inline (dynamic_item_keyboard);

  function toolbar_item
    (parent  : item_access_t;
     toolbar : agar.gui.widget.toolbar.toolbar_access_t;
     text    : string;
     icon    : agar.gui.surface.surface_access_t;
     key     : c.int;
     modkey  : c.int;
     func    : agar.core.event.callback_t) return item_access_t;
  pragma inline (toolbar_item);

  procedure set_icon
    (item    : item_access_t;
     surface : agar.gui.surface.surface_access_t);
  pragma import (c, set_icon, "AG_MenuSetIcon");

  procedure set_label
    (item  : item_access_t;
     label : string);
  pragma inline (set_label);

  procedure set_poll_function
    (item  : item_access_t;
     func  : agar.core.event.callback_t);
  pragma inline (set_poll_function);

  procedure deallocate_item (item : item_access_t);
  pragma import (c, deallocate_item, "AG_MenuItemFree");

  procedure item_state
    (item  : item_access_t;
     state : state_t);
  pragma import (c, item_state, "AG_MenuState");

  procedure enable (item : item_access_t);
  pragma import (c, enable, "AG_MenuEnable");

  procedure disable (item : item_access_t);
  pragma import (c, disable, "AG_MenuDisable");

  -- boolean and bitmask items

  function bind_bool
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access boolean;
     invert : boolean) return item_access_t;
  pragma inline (bind_bool);

  function bind_bool_with_mutex
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access boolean;
     invert : boolean;
     mutex  : agar.core.threads.mutex_t) return item_access_t;
  pragma inline (bind_bool_with_mutex);

  function bind_flags
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask_t;
     flags  : mask_t;
     invert : boolean) return item_access_t;
  pragma inline (bind_flags);

  function bind_flags_with_mutex
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask_t;
     flags  : mask_t;
     invert : boolean;
     mutex  : agar.core.threads.mutex_t) return item_access_t;
  pragma inline (bind_flags_with_mutex);

  function bind_flags8
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask8_t;
     flags  : mask8_t;
     invert : boolean) return item_access_t;
  pragma inline (bind_flags8);

  function bind_flags8_with_mutex
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask8_t;
     flags  : mask8_t;
     invert : boolean;
     mutex  : agar.core.threads.mutex_t) return item_access_t;
  pragma inline (bind_flags8_with_mutex);

  function bind_flags16
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask16_t;
     flags  : mask16_t;
     invert : boolean) return item_access_t;
  pragma inline (bind_flags16);

  function bind_flags16_with_mutex
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask16_t;
     flags  : mask16_t;
     invert : boolean;
     mutex  : agar.core.threads.mutex_t) return item_access_t;
  pragma inline (bind_flags16_with_mutex);

  function bind_flags32
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask32_t;
     flags  : mask32_t;
     invert : boolean) return item_access_t;
  pragma inline (bind_flags32);

  function bind_flags32_with_mutex
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask32_t;
     flags  : mask32_t;
     invert : boolean;
     mutex  : agar.core.threads.mutex_t) return item_access_t;
  pragma inline (bind_flags32_with_mutex);

  -- other items

  procedure separator (item : item_access_t);
  pragma import (c, separator, "AG_MenuSeparator");

  procedure section
    (item : item_access_t;
     text : string);
  pragma inline (section);

  -- popup menus

  package popup is
    function allocate (widget : widget_access_t) return popup_menu_access_t;
    pragma import (c, allocate, "AG_PopupNew");

    procedure show_at
      (menu : popup_menu_access_t;
       x    : natural;
       y    : natural);
    pragma inline (show_at);

    procedure hide (menu : popup_menu_access_t);
    pragma import (c, hide, "AG_PopupHide");

    procedure destroy
      (widget : widget_access_t;
       menu   : popup_menu_access_t);
    pragma import (c, destroy, "AG_PopupDestroy");
  end popup;

  -- widget casts

  function widget (menu : menu_access_t) return widget_access_t;
  pragma inline (widget);

  function widget (view : view_access_t) return widget_access_t;
  pragma inline (widget);

end agar.gui.widget.menu;
