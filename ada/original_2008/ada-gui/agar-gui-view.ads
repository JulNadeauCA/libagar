with agar.core.event;
with agar.core.object;
with agar.core.tail_queue;
with agar.core;
with agar.gui.style;
with agar.gui.surface;
with agar.gui.widget;
with agar.gui.window;
with interfaces.c;
with sdl.keysym;
with sdl.video;

package agar.gui.view is
  package c renames interfaces.c;

  package window_tail_queue is new agar.core.tail_queue
    (entry_type => agar.gui.window.window_access_t);

  type winop_t is (
    WINOP_NONE,
    WINOP_MOVE,
    WINOP_LRESIZE,
    WINOP_RRESIZE,
    WINOP_HRESIZE
  );
  for winop_t use (
    WINOP_NONE    => 0,
    WINOP_MOVE    => 1,
    WINOP_LRESIZE => 2,
    WINOP_RRESIZE => 3,
    WINOP_HRESIZE => 4
  );
  for winop_t'size use c.unsigned'size;
  pragma convention (c, winop_t);

  type display_t is record
    object       : agar.core.object.object_t;
    v            : sdl.video.surface_access_t;
    stmpl        : agar.gui.surface.surface_access_t;
    w            : c.int;
    h            : c.int;
    depth        : c.int;
    opengl       : c.int;
    rcur         : c.int;
    rnom         : c.unsigned;
    dirty        : sdl.video.rect_access_t;
    ndirty       : c.unsigned;
    maxdirty     : c.unsigned;
    windows      : window_tail_queue.head_t;
    detach       : window_tail_queue.head_t;
    win_to_focus : agar.gui.window.window_access_t;
    win_selected : agar.gui.window.window_access_t;
    win_modal    : access agar.gui.window.window_access_t;
    nmodal       : c.unsigned;
    winop        : winop_t;
    style        : agar.gui.style.style_access_t;
  end record;
  type display_access_t is access all display_t;
  pragma convention (c, display_t);
  pragma convention (c, display_access_t);

  function resize_display
    (width  : positive;
     height : positive) return boolean;
  pragma inline (resize_display);

  function set_refresh_rate (rate : positive) return boolean;
  pragma inline (set_refresh_rate);

  procedure bind_global_key
    (key    : sdl.keysym.key_t;
     modkey : sdl.keysym.modkey_t;
     proc : access procedure);
  pragma import (c, bind_global_key, "AG_BindGlobalKey");

  procedure bind_global_key_event
    (key    : sdl.keysym.key_t;
     modkey : sdl.keysym.modkey_t;
     proc : access procedure (ev : agar.core.event.event_access_t));
  pragma import (c, bind_global_key_event, "AG_BindGlobalKeyEv");

  function unbind_global_key
    (key    : sdl.keysym.key_t;
     modkey : sdl.keysym.modkey_t) return c.int;
  pragma import (c, unbind_global_key, "AG_UnbindGlobalKey");

  function unbind_global_key
    (key    : sdl.keysym.key_t;
     modkey : sdl.keysym.modkey_t) return boolean;
  pragma inline (unbind_global_key);
 
  procedure clear_global_keys;
  pragma import (c, clear_global_keys, "AG_ClearGlobalKeys");

  function find_window (name : string) return agar.gui.window.window_access_t;
  pragma inline (find_window);
 
  function find_widget
    (view : display_access_t;
     name : string) return agar.gui.widget.widget_access_t;
  pragma inline (find_widget);
 
end agar.gui.view;
