package body agar.gui.view is

  use type c.int;

  function init_video
    (width  : positive;
     height : positive;
     bpp    : natural;
     flags  : video_flags_t) return boolean is
  begin
    return init_video
      (width  => c.int (width),
       height => c.int (height),
       bpp    => c.int (bpp),
       flags  => flags) = 0;
  end init_video;

  function resize_display
    (width  : positive;
     height : positive) return boolean is
  begin
    return resize_display
      (width  => c.int (width),
       height => c.int (height)) = 0;
  end resize_display;

  function set_refresh_rate (rate : positive) return boolean is
  begin
    return set_refresh_rate (c.int (rate)) = 0;
  end set_refresh_rate;

  function unbind_global_key
    (key    : sdl.keysym.key_t;
     modkey : sdl.keysym.modkey_t) return boolean is
  begin
    return unbind_global_key (key, modkey) = 0;
  end unbind_global_key;

  function find_window (name : string) return agar.gui.window.window_access_t is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return find_window (cs.to_chars_ptr (ca_name'unchecked_access));
  end find_window;

  function find_widget
    (view : display_access_t;
     name : string) return agar.gui.widget.widget_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return find_widget (view, cs.to_chars_ptr (ca_name'unchecked_access));
  end find_widget;

end agar.gui.view;
