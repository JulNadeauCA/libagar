package body agar.gui.widget.tlist is

  package cbinds is
    procedure set_item_height
      (tlist  : tlist_access_t;
       height : c.int);
    pragma import (c, set_item_height, "AG_TlistSetItemHeight");

    procedure size_hint
      (tlist     : tlist_access_t;
       text      : cs.chars_ptr;
       num_items : c.int);
    pragma import (c, size_hint, "AG_TlistSizeHint");

    procedure size_hint_pixels
      (tlist     : tlist_access_t;
       width     : c.int;
       num_items : c.int);
    pragma import (c, size_hint_pixels, "AG_TlistSizeHintPixels");

    procedure size_hint_largest
      (tlist     : tlist_access_t;
       num_items : c.int);
    pragma import (c, size_hint_largest, "AG_TlistSizeHintLargest");

    procedure set_double_click_callback
      (tlist     : tlist_access_t;
       callback  : agar.core.event.callback_t;
       fmt       : agar.core.types.void_ptr_t);
    pragma import (c, set_double_click_callback, "AG_TlistSetDblClickFn");

    procedure set_changed_callback
      (tlist     : tlist_access_t;
       callback  : agar.core.event.callback_t;
       fmt       : agar.core.types.void_ptr_t);
    pragma import (c, set_changed_callback, "AG_TlistSetChangedFn");

    function list_select_text
      (tlist : tlist_access_t;
       text  : cs.chars_ptr) return item_access_t;
    pragma import (c, list_select_text, "AG_TlistSelectText");

    function list_find_by_index
      (tlist : tlist_access_t;
       index : c.int) return item_access_t;
    pragma import (c, list_find_by_index, "AG_TlistFindByIndex");

    function set_popup_callback
      (tlist    : tlist_access_t;
       callback : agar.core.event.callback_t;
       fmt      : agar.core.types.void_ptr_t) return agar.gui.widget.menu.item_access_t;
    pragma import (c, set_popup_callback, "AG_TlistSetPopupFn");

    function set_popup
      (tlist    : tlist_access_t;
       category : cs.chars_ptr) return agar.gui.widget.menu.item_access_t;
    pragma import (c, set_popup, "AG_TlistSetPopup");
  end cbinds;

  procedure set_item_height
    (tlist  : tlist_access_t;
     height : natural) is
  begin
    cbinds.set_item_height
      (tlist  => tlist,
       height => c.int (height));
  end set_item_height;

  procedure size_hint
    (tlist     : tlist_access_t;
     text      : string;
     num_items : natural)
  is
    c_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.size_hint
      (tlist     => tlist,
       text      => cs.to_chars_ptr (c_text'unchecked_access),
       num_items => c.int (num_items));
  end size_hint;

  procedure size_hint_pixels
    (tlist     : tlist_access_t;
     width     : natural;
     num_items : natural) is
  begin
    cbinds.size_hint_pixels
      (tlist     => tlist,
       width     => c.int (width),
       num_items => c.int (num_items));
  end size_hint_pixels;

  procedure size_hint_largest
    (tlist     : tlist_access_t;
     num_items : natural) is
  begin
    cbinds.size_hint_largest
      (tlist     => tlist,
       num_items => c.int (num_items));
  end size_hint_largest;

  procedure set_double_click_callback
    (tlist     : tlist_access_t;
     callback  : agar.core.event.callback_t) is
  begin
    cbinds.set_double_click_callback
      (tlist    => tlist,
       callback => callback,
       fmt      => agar.core.types.null_ptr);
  end set_double_click_callback;

  procedure set_changed_callback
    (tlist     : tlist_access_t;
     callback  : agar.core.event.callback_t) is
  begin
    cbinds.set_changed_callback
      (tlist    => tlist,
       callback => callback,
       fmt      => agar.core.types.null_ptr);
  end set_changed_callback;

  function list_select_text
    (tlist : tlist_access_t;
     text  : string) return item_access_t
  is
    c_text : aliased c.char_array := c.to_c (text);
  begin
    return cbinds.list_select_text
      (tlist => tlist,
       text  => cs.to_chars_ptr (c_text'unchecked_access));
  end list_select_text;

  function list_find_by_index
    (tlist : tlist_access_t;
     index : integer) return item_access_t is
  begin
    return cbinds.list_find_by_index
      (tlist => tlist,
       index => c.int (index));
  end list_find_by_index;

  function set_popup_callback
    (tlist    : tlist_access_t;
     callback : agar.core.event.callback_t) return agar.gui.widget.menu.item_access_t is
  begin
    return cbinds.set_popup_callback
      (tlist    => tlist,
       callback => callback,
       fmt      => agar.core.types.null_ptr);
  end set_popup_callback;

  function set_popup
    (tlist    : tlist_access_t;
     category : string) return agar.gui.widget.menu.item_access_t
  is
    c_cat : aliased c.char_array := c.to_c (category);
  begin
    return cbinds.set_popup
      (tlist    => tlist,
       category => cs.to_chars_ptr (c_cat'unchecked_access));
  end set_popup;

  function widget (tlist : tlist_access_t) return widget_access_t is
  begin
    return tlist.widget'access;
  end widget;

end agar.gui.widget.tlist;
