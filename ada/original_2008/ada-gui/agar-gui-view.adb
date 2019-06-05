with interfaces.c.strings;

package body agar.gui.view is
  package cs renames interfaces.c.strings;

  use type c.int;

  package cbinds is
    function resize_display
      (width  : c.int;
       height : c.int) return c.int;
    pragma import (c, resize_display, "AG_ResizeDisplay");
  
    function set_refresh_rate (rate : c.int) return c.int;
    pragma import (c, set_refresh_rate, "AG_SetRefreshRate");
  
    function find_window (name : cs.chars_ptr) return agar.gui.window.window_access_t;
    pragma import (c, find_window, "AG_FindWindow");
  
    function find_widget
      (view : display_access_t;
       name : cs.chars_ptr) return agar.gui.widget.widget_access_t;
    pragma import (c, find_widget, "AG_WidgetFind");

    function unbind_global_key
      (key    : sdl.keysym.key_t;
       modkey : sdl.keysym.modkey_t) return c.int;
    pragma import (c, unbind_global_key, "AG_UnbindGlobalKey");
  end cbinds;

  function resize_display
    (width  : positive;
     height : positive) return boolean is
  begin
    return cbinds.resize_display
      (width  => c.int (width),
       height => c.int (height)) = 0;
  end resize_display;

  function set_refresh_rate (rate : positive) return boolean is
  begin
    return cbinds.set_refresh_rate (c.int (rate)) = 0;
  end set_refresh_rate;

  function unbind_global_key
    (key    : sdl.keysym.key_t;
     modkey : sdl.keysym.modkey_t) return boolean is
  begin
    return cbinds.unbind_global_key (key, modkey) = 0;
  end unbind_global_key;

  function find_window (name : string) return agar.gui.window.window_access_t is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return cbinds.find_window (cs.to_chars_ptr (ca_name'unchecked_access));
  end find_window;

  function find_widget
    (view : display_access_t;
     name : string) return agar.gui.widget.widget_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return cbinds.find_widget (view, cs.to_chars_ptr (ca_name'unchecked_access));
  end find_widget;

end agar.gui.view;
