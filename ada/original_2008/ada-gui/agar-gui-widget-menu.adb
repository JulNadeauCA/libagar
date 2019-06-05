package body agar.gui.widget.menu is

  use type c.int;

  package cbinds is
    procedure expand
      (menu : menu_access_t;
       item : item_access_t;
       x    : c.int;
       y    : c.int);
    pragma import (c, expand, "AG_MenuExpand"); 

    procedure set_padding
      (menu   : menu_access_t;
       left   : c.int;
       right  : c.int;
       top    : c.int;
       bottom : c.int);
    pragma import (c, set_padding, "AG_MenuSetPadding"); 

    procedure set_label_padding
      (menu   : menu_access_t;
       left   : c.int;
       right  : c.int;
       top    : c.int;
       bottom : c.int);
    pragma import (c, set_label_padding, "AG_MenuSetLabelPadding"); 

    function node
      (parent : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t) return item_access_t;
    pragma import (c, node, "AG_MenuNode");

    function action
      (parent : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       func   : agar.core.event.callback_t;
       fmt    : agar.core.types.void_ptr_t) return item_access_t;
    pragma import (c, action, "AG_MenuAction"); 

    function action_keyboard
      (parent : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       key    : c.int;
       modkey : c.int;
       func   : agar.core.event.callback_t;
       fmt    : agar.core.types.void_ptr_t) return item_access_t;
    pragma import (c, action_keyboard, "AG_MenuActionKb"); 

    function dynamic_item
      (parent : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       func   : agar.core.event.callback_t;
       fmt    : agar.core.types.void_ptr_t) return item_access_t;
    pragma import (c, dynamic_item, "AG_MenuDynamicItem");

    function dynamic_item_keyboard
      (parent : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       key    : c.int;
       modkey : c.int;
       func   : agar.core.event.callback_t;
       fmt    : agar.core.types.void_ptr_t) return item_access_t;
    pragma import (c, dynamic_item_keyboard, "AG_MenuDynamicItemKb");

    function toolbar_item
      (parent  : item_access_t;
       toolbar : agar.gui.widget.toolbar.toolbar_access_t;
       text    : cs.chars_ptr;
       icon    : agar.gui.surface.surface_access_t;
       key     : c.int;
       modkey  : c.int;
       func    : agar.core.event.callback_t;
       fmt     : agar.core.types.void_ptr_t) return item_access_t;
    pragma import (c, toolbar_item, "AG_MenuTool");

    procedure set_label
      (item  : item_access_t;
       label : cs.chars_ptr);
    pragma import (c, set_label, "AG_MenuSetLabelS");

    procedure set_poll_function
      (item  : item_access_t;
       func  : agar.core.event.callback_t;
       fmt   : agar.core.types.void_ptr_t);
    pragma import (c, set_poll_function, "AG_MenuSetPollFn");

    function bind_bool
      (item   : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       value  : access c.int;
       invert : c.int) return item_access_t;
    pragma import (c, bind_bool, "agar_gui_widget_menu_bool");

    function bind_bool_with_mutex
      (item   : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       value  : access c.int;
       invert : c.int;
       mutex  : agar.core.threads.mutex_t) return item_access_t;
    pragma import (c, bind_bool_with_mutex, "AG_MenuIntBoolMp");

    function bind_flags
      (item   : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       value  : access mask_t;
       flags  : mask_t;
       invert : c.int) return item_access_t;
    pragma import (c, bind_flags, "agar_gui_widget_menu_int_flags");

    function bind_flags_with_mutex
      (item   : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       value  : access mask_t;
       flags  : mask_t;
       invert : c.int;
       mutex  : agar.core.threads.mutex_t) return item_access_t;
    pragma import (c, bind_flags_with_mutex, "AG_MenuIntFlagsMp");

    function bind_flags8
      (item   : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       value  : access mask8_t;
       flags  : mask8_t;
       invert : c.int) return item_access_t;
    pragma import (c, bind_flags8, "agar_gui_widget_menu_int_flags8");

    function bind_flags8_with_mutex
      (item   : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       value  : access mask8_t;
       flags  : mask8_t;
       invert : c.int;
       mutex  : agar.core.threads.mutex_t) return item_access_t;
    pragma import (c, bind_flags8_with_mutex, "AG_MenuInt8FlagsMp");

    function bind_flags16
      (item   : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       value  : access mask16_t;
       flags  : mask16_t;
       invert : c.int) return item_access_t;
    pragma import (c, bind_flags16, "agar_gui_widget_menu_int_flags16");

    function bind_flags16_with_mutex
      (item   : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       value  : access mask16_t;
       flags  : mask16_t;
       invert : c.int;
       mutex  : agar.core.threads.mutex_t) return item_access_t;
    pragma import (c, bind_flags16_with_mutex, "AG_MenuInt16FlagsMp");

    function bind_flags32
      (item   : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       value  : access mask32_t;
       flags  : mask32_t;
       invert : c.int) return item_access_t;
    pragma import (c, bind_flags32, "agar_gui_widget_menu_int_flags32");

    function bind_flags32_with_mutex
      (item   : item_access_t;
       text   : cs.chars_ptr;
       icon   : agar.gui.surface.surface_access_t;
       value  : access mask32_t;
       flags  : mask32_t;
       invert : c.int;
       mutex  : agar.core.threads.mutex_t) return item_access_t;
    pragma import (c, bind_flags32_with_mutex, "AG_MenuInt32FlagsMp");

    procedure section
      (item : item_access_t;
       text : cs.chars_ptr);
    pragma import (c, section, "AG_MenuSectionS");
  end cbinds;

  procedure expand
    (menu : menu_access_t;
     item : item_access_t;
     x    : natural;
     y    : natural) is
  begin
    cbinds.expand
      (menu => menu,
       item => item,
       x    => c.int (x),
       y    => c.int (y));
  end expand;
 
  procedure set_padding
    (menu   : menu_access_t;
     left   : natural;
     right  : natural;
     top    : natural;
     bottom : natural) is
  begin
    cbinds.set_padding
      (menu   => menu,
       left   => c.int (left),
       right  => c.int (right),
       top    => c.int (top),
       bottom => c.int (bottom));
  end set_padding;
 
  procedure set_label_padding
    (menu   : menu_access_t;
     left   : natural;
     right  : natural;
     top    : natural;
     bottom : natural) is
  begin
    cbinds.set_label_padding
      (menu   => menu,
       left   => c.int (left),
       right  => c.int (right),
       top    => c.int (top),
       bottom => c.int (bottom));
  end set_label_padding;
 
  function node
    (parent : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t) return item_access_t
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    return cbinds.node
      (parent => parent,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon);
  end node;

  function action
    (parent : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     func   : agar.core.event.callback_t) return item_access_t
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    return cbinds.action
      (parent => parent,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       func   => func,
       fmt    => agar.core.types.null_ptr);
  end action;
 
  function action_keyboard
    (parent : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     key    : c.int;
     modkey : c.int;
     func   : agar.core.event.callback_t) return item_access_t
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    return cbinds.action_keyboard
      (parent => parent,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       key    => key,
       modkey => modkey,
       func   => func,
       fmt    => agar.core.types.null_ptr);
  end action_keyboard;
 
  function dynamic_item
    (parent : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     func   : agar.core.event.callback_t) return item_access_t
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    return cbinds.dynamic_item
      (parent => parent,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       func   => func,
       fmt    => agar.core.types.null_ptr);
  end dynamic_item;

  function dynamic_item_keyboard
    (parent : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     key    : c.int;
     modkey : c.int;
     func   : agar.core.event.callback_t) return item_access_t
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    return cbinds.dynamic_item_keyboard
      (parent => parent,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       key    => key,
       modkey => modkey,
       func   => func,
       fmt    => agar.core.types.null_ptr);
  end dynamic_item_keyboard;

  function toolbar_item
    (parent  : item_access_t;
     toolbar : agar.gui.widget.toolbar.toolbar_access_t;
     text    : string;
     icon    : agar.gui.surface.surface_access_t;
     key     : c.int;
     modkey  : c.int;
     func    : agar.core.event.callback_t) return item_access_t
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    return cbinds.toolbar_item
      (parent  => parent,
       toolbar => toolbar,
       text    => cs.to_chars_ptr (ca_text'unchecked_access),
       icon    => icon,
       key     => key,
       modkey  => modkey,
       func    => func,
       fmt     => agar.core.types.null_ptr);
  end toolbar_item;

  procedure set_label
    (item  : item_access_t;
     label : string)
  is
    ca_label : aliased c.char_array := c.to_c (label);
  begin
    cbinds.set_label
      (item  => item,
       label => cs.to_chars_ptr (ca_label'unchecked_access));
  end set_label;

  procedure set_poll_function
    (item  : item_access_t;
     func  : agar.core.event.callback_t) is
  begin
    cbinds.set_poll_function
      (item => item,
       func => func,
       fmt  => agar.core.types.null_ptr);
  end set_poll_function;

  function bind_bool
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access boolean;
     invert : boolean) return item_access_t
  is
    ca_text  : aliased c.char_array := c.to_c (text);
    c_invert : c.int := 0;
    c_value  : aliased c.int;
    item_acc : item_access_t;
  begin
    if invert then c_invert := 1; end if;
    item_acc := cbinds.bind_bool
      (item   => item,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       value  => c_value'unchecked_access,
       invert => c_invert);
    value.all := c_value = 1;
    return item_acc;
  end bind_bool;

  function bind_bool_with_mutex
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access boolean;
     invert : boolean;
     mutex  : agar.core.threads.mutex_t) return item_access_t
  is
    ca_text  : aliased c.char_array := c.to_c (text);
    c_invert : c.int := 0;
    c_value  : aliased c.int;
    item_acc : item_access_t;
  begin
    if invert then c_invert := 1; end if;
    item_acc := cbinds.bind_bool_with_mutex
      (item   => item,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       value  => c_value'unchecked_access,
       invert => c_invert,
       mutex  => mutex);
    value.all := c_value = 1;
    return item_acc;
  end bind_bool_with_mutex;

  function bind_flags
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask_t;
     flags  : mask_t;
     invert : boolean) return item_access_t
  is
    ca_text  : aliased c.char_array := c.to_c (text);
    c_invert : c.int := 0;
  begin
    if invert then c_invert := 1; end if;
    return cbinds.bind_flags
      (item   => item,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       value  => value,
       flags  => flags,
       invert => c_invert);
  end bind_flags;

  function bind_flags_with_mutex
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask_t;
     flags  : mask_t;
     invert : boolean;
     mutex  : agar.core.threads.mutex_t) return item_access_t
  is
    ca_text  : aliased c.char_array := c.to_c (text);
    c_invert : c.int := 0;
  begin
    if invert then c_invert := 1; end if;
    return cbinds.bind_flags_with_mutex
      (item   => item,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       value  => value,
       flags  => flags,
       invert => c_invert,
       mutex  => mutex);
  end bind_flags_with_mutex;
 
  function bind_flags8
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask8_t;
     flags  : mask8_t;
     invert : boolean) return item_access_t
  is
    ca_text  : aliased c.char_array := c.to_c (text);
    c_invert : c.int := 0;
  begin
    if invert then c_invert := 1; end if;
    return cbinds.bind_flags8
      (item   => item,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       value  => value,
       flags  => flags,
       invert => c_invert);
  end bind_flags8;

  function bind_flags8_with_mutex
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask8_t;
     flags  : mask8_t;
     invert : boolean;
     mutex  : agar.core.threads.mutex_t) return item_access_t
  is
    ca_text  : aliased c.char_array := c.to_c (text);
    c_invert : c.int := 0; 
  begin
    if invert then c_invert := 1; end if;
    return cbinds.bind_flags8_with_mutex
      (item   => item,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       value  => value,
       flags  => flags,
       invert => c_invert,
       mutex  => mutex);
  end bind_flags8_with_mutex;
 
  function bind_flags16
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask16_t;
     flags  : mask16_t;
     invert : boolean) return item_access_t
  is
    ca_text  : aliased c.char_array := c.to_c (text);
    c_invert : c.int := 0;
  begin
    if invert then c_invert := 1; end if;
    return cbinds.bind_flags16
      (item   => item,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       value  => value,
       flags  => flags,
       invert => c_invert);
  end bind_flags16;

  function bind_flags16_with_mutex
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask16_t;
     flags  : mask16_t;
     invert : boolean;
     mutex  : agar.core.threads.mutex_t) return item_access_t
  is
    ca_text  : aliased c.char_array := c.to_c (text);
    c_invert : c.int := 0;
  begin
    if invert then c_invert := 1; end if;
    return cbinds.bind_flags16_with_mutex
      (item   => item,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       value  => value,
       flags  => flags,
       invert => c_invert,
       mutex  => mutex);
  end bind_flags16_with_mutex;

  function bind_flags32
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask32_t;
     flags  : mask32_t;
     invert : boolean) return item_access_t
  is
    ca_text  : aliased c.char_array := c.to_c (text);
    c_invert : c.int := 0;
  begin
    if invert then c_invert := 1; end if;
    return cbinds.bind_flags32
      (item   => item,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       value  => value,
       flags  => flags,
       invert => c_invert);
  end bind_flags32;

  function bind_flags32_with_mutex
    (item   : item_access_t;
     text   : string;
     icon   : agar.gui.surface.surface_access_t;
     value  : access mask32_t;
     flags  : mask32_t;
     invert : boolean;
     mutex  : agar.core.threads.mutex_t) return item_access_t
  is
    ca_text  : aliased c.char_array := c.to_c (text);
    c_invert : c.int := 0;
  begin
    if invert then c_invert := 1; end if;
    return cbinds.bind_flags32_with_mutex
      (item   => item,
       text   => cs.to_chars_ptr (ca_text'unchecked_access),
       icon   => icon,
       value  => value,
       flags  => flags,
       invert => c_invert,
       mutex  => mutex);
  end bind_flags32_with_mutex;
 
  procedure section
    (item : item_access_t;
     text : string)
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.section
      (item => item,
       text => cs.to_chars_ptr (ca_text'unchecked_access));
  end section;
 
  -- popup menus
  package body popup is
    package cbinds is
      procedure show_at
        (menu : popup_menu_access_t;
         x    : c.int;
         y    : c.int);
      pragma import (c, show_at, "AG_PopupShowAt");
    end cbinds;

    procedure show_at
      (menu : popup_menu_access_t;
       x    : natural;
       y    : natural) is
    begin
      cbinds.show_at
        (menu => menu,
         x    => c.int (x),
         y    => c.int (y));
    end show_at;
  end popup;

  function widget (menu : menu_access_t) return widget_access_t is
  begin
    return menu.widget'access;
  end widget;

  function widget (view : view_access_t) return widget_access_t is
  begin
    return view.widget'access;
  end widget;

end agar.gui.widget.menu;
