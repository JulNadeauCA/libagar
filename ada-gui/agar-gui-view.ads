with agar.core.event;
with agar.core.object;
with agar.core.tail_queue;
with agar.core.types;
with agar.core;
with agar.gui.surface;
with agar.gui.widget;
with agar.gui.window;
with interfaces.c.strings;
with interfaces.c;
with sdl.keysym;
with sdl.video;

package agar.gui.view is
  package c renames interfaces.c;
  package cs renames interfaces.c.strings;

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
    v            : sdl.video.surface_ptr_t;
    stmpl        : agar.gui.surface.surface_access_t;
    w            : c.int;
    h            : c.int;
    depth        : c.int;
    opengl       : c.int;
    rcur         : c.int;
    rnom         : c.unsigned;
    dirty        : sdl.video.rect_ptr_t;
    ndirty       : c.unsigned;
    maxdirty     : c.unsigned;
    windows      : window_tail_queue.head_t;
    detach       : window_tail_queue.head_t;
    win_to_focus : agar.gui.window.window_access_t;
    win_selected : agar.gui.window.window_access_t;
    win_modal    : access agar.gui.window.window_access_t;
    nmodal       : c.unsigned;
    winop        : winop_t;
    style        : agar.core.types.void_ptr_t; -- XXX: style_access_t
  end record;
  type display_access_t is access all display_t;
  pragma convention (c, display_t);
  pragma convention (c, display_access_t);

  -- Flags for init_video
  subtype video_flags_t is c.unsigned;
  HWSURFACE     : constant video_flags_t := 16#001#;
  ASYNCBLIT     : constant video_flags_t := 16#002#;
  ANYFORMAT     : constant video_flags_t := 16#004#;
  HWPALETTE     : constant video_flags_t := 16#008#;
  DOUBLEBUF     : constant video_flags_t := 16#010#;
  FULLSCREEN    : constant video_flags_t := 16#020#;
  RESIZABLE     : constant video_flags_t := 16#040#;
  NOFRAME       : constant video_flags_t := 16#080#;
  BGPOPUPMENU   : constant video_flags_t := 16#100#;
  OPENGL        : constant video_flags_t := 16#200#;
  OPENGL_OR_SDL : constant video_flags_t := 16#400#;
  NOBGCLEAR     : constant video_flags_t := 16#800#;

  function init_video
    (width  : c.int;
     height : c.int;
     bpp    : c.int;
     flags  : video_flags_t := 0) return c.int;
  pragma import (c, init_video, "AG_InitVideo");

  function init_video
    (width  : positive;
     height : positive;
     bpp    : natural;
     flags  : video_flags_t := 0) return boolean;
  pragma inline (init_video);

  function resize_display
    (width  : c.int;
     height : c.int) return c.int;
  pragma import (c, resize_display, "AG_ResizeDisplay");

  function resize_display
    (width  : positive;
     height : positive) return boolean;
  pragma inline (resize_display);

  function set_refresh_rate (rate : c.int) return c.int;
  pragma import (c, set_refresh_rate, "AG_SetRefreshRate");

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

  procedure attach (window : agar.gui.window.window_access_t);
  pragma import (c, attach, "AG_ViewAttach");

  procedure detach (window : agar.gui.window.window_access_t);
  pragma import (c, detach, "AG_ViewDetach");

  function find_window (name : cs.chars_ptr) return agar.gui.window.window_access_t;
  pragma import (c, find_window, "AG_FindWindow");

  function find_window (name : string) return agar.gui.window.window_access_t;
  pragma inline (find_window);
 
  function find_widget
    (view : display_access_t;
     name : cs.chars_ptr) return agar.gui.widget.widget_access_t;
  pragma import (c, find_widget, "AG_WidgetFind");

  function find_widget
    (view : display_access_t;
     name : string) return agar.gui.widget.widget_access_t;
  pragma inline (find_widget);
 
end agar.gui.view;
