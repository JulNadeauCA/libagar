package body agar.gui.widget.radio is

  use type c.int;

  package cbinds is
    function allocate
      (parent : widget_access_t;
       flags  : flags_t;
       items  : agar.core.types.void_ptr_t) return radio_access_t;
    pragma import (c, allocate, "AG_RadioNew");

    function add_item
      (radio : radio_access_t;
       text  : cs.chars_ptr) return c.int;
    pragma import (c, add_item, "AG_RadioAddItemS");

    function add_item_hotkey
      (radio : radio_access_t;
       key   : c.int;
       text  : cs.chars_ptr) return c.int;
    pragma import (c, add_item_hotkey, "AG_RadioAddItemHKS");
  end cbinds;

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return radio_access_t is
  begin
    return cbinds.allocate
      (parent => parent,
       flags  => flags,
       items  => agar.core.types.null_ptr);
  end allocate;

  function add_item
    (radio : radio_access_t;
     text  : string) return boolean
  is
    c_txt : aliased c.char_array := c.to_c (text);
  begin
    return cbinds.add_item
      (radio => radio,
       text  => cs.to_chars_ptr (c_txt'unchecked_access)) = 0;
  end add_item;

  function add_item_hotkey
    (radio : radio_access_t;
     key   : c.int;
     text  : string) return boolean
  is
    c_txt : aliased c.char_array := c.to_c (text);
  begin
    return cbinds.add_item_hotkey
      (radio => radio,
       key   => key,
       text  => cs.to_chars_ptr (c_txt'unchecked_access)) = 0;
  end add_item_hotkey;

  function widget (radio : radio_access_t) return widget_access_t is
  begin
    return radio.widget'access;
  end widget;

end agar.gui.widget.radio;
