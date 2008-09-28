with agar.gui.surface;
with agar.gui.widget;
with agar.gui.types;

package agar.gui.window is

  use type c.unsigned;
  use type agar.gui.types.window_flags_t;

  subtype window_t is agar.gui.types.window_t;
  subtype window_access_t is agar.gui.types.window_access_t;

  --
  -- constants
  --

  caption_max : constant c.unsigned := agar.gui.types.window_caption_max;

  --
  -- types
  --

  subtype flags_t is agar.gui.types.window_flags_t;
  WINDOW_MODAL         : constant flags_t := 16#00001#;
  WINDOW_MAXIMIZED     : constant flags_t := 16#00002#;
  WINDOW_MINIMIZED     : constant flags_t := 16#00004#;
  WINDOW_KEEPABOVE     : constant flags_t := 16#00008#;
  WINDOW_KEEPBELOW     : constant flags_t := 16#00010#;
  WINDOW_DENYFOCUS     : constant flags_t := 16#00020#;
  WINDOW_NOTITLE       : constant flags_t := 16#00040#;
  WINDOW_NOBORDERS     : constant flags_t := 16#00080#;
  WINDOW_NOHRESIZE     : constant flags_t := 16#00100#;
  WINDOW_NOVRESIZE     : constant flags_t := 16#00200#;
  WINDOW_NOCLOSE       : constant flags_t := 16#00400#;
  WINDOW_NOMINIMIZE    : constant flags_t := 16#00800#;
  WINDOW_NOMAXIMIZE    : constant flags_t := 16#01000#;
  WINDOW_NOBACKGROUND  : constant flags_t := 16#08000#;
  WINDOW_NOUPDATERECT  : constant flags_t := 16#10000#;
  WINDOW_FOCUSONATTACH : constant flags_t := 16#20000#;
  WINDOW_HMAXIMIZE     : constant flags_t := 16#40000#;
  WINDOW_VMAXIMIZE     : constant flags_t := 16#80000#;
  WINDOW_NORESIZE      : constant flags_t := WINDOW_NOHRESIZE or WINDOW_NOVRESIZE;
  WINDOW_PLAIN         : constant flags_t := WINDOW_NOTITLE or WINDOW_NOBORDERS;

  subtype alignment_t is agar.gui.types.window_alignment_t;

  type close_action_t is (
    WINDOW_HIDE,
    WINDOW_DETACH,
    WINDOW_NONE
  );
  for close_action_t use (
    WINDOW_HIDE   => 0,
    WINDOW_DETACH => 1,
    WINDOW_NONE   => 2
  );
  for close_action_t'size use c.unsigned'size;
  pragma convention (c, close_action_t);

  subtype percent_t is positive range 1 .. 100;

  --
  -- API
  --

  function allocate (flags : flags_t := 0) return window_access_t;
  pragma import (c, allocate, "AG_WindowNew");

  function allocate_named
    (flags : flags_t := 0;
     name  : string) return window_access_t;
  pragma inline (allocate_named);

  procedure set_caption
    (window  : window_access_t;
     caption : string);
  pragma inline (set_caption);

  procedure set_icon
    (window  : window_access_t;
     surface : agar.gui.surface.surface_access_t);
  pragma import (c, set_icon, "agar_window_set_icon");

  procedure set_icon_no_copy
    (window  : window_access_t;
     surface : agar.gui.surface.surface_access_t);
  pragma import (c, set_icon_no_copy, "agar_window_set_icon_no_copy");

  procedure set_close_action
    (window : window_access_t;
     mode   : close_action_t);
  pragma import (c, set_close_action, "AG_WindowSetCloseAction");

  procedure set_padding
    (window : window_access_t;
     left   : natural;
     right  : natural;
     top    : natural;
     bottom : natural);
  pragma inline (set_padding);

  procedure set_spacing
    (window  : window_access_t;
     spacing : natural);
  pragma inline (set_spacing);
 
  procedure set_position
    (window    : window_access_t;
     alignment : alignment_t;
     cascade   : boolean);
  pragma inline (set_position);

  procedure set_geometry
    (window : window_access_t;
     x      : natural;
     y      : natural;
     width  : natural;
     height : natural);
  pragma inline (set_geometry);
 
  procedure set_geometry_aligned
    (window    : window_access_t;
     alignment : alignment_t;
     width     : positive;
     height    : positive);
  pragma inline (set_geometry_aligned);
 
  procedure set_geometry_aligned_percent
    (window    : window_access_t;
     alignment : alignment_t;
     width     : percent_t;
     height    : percent_t);
  pragma inline (set_geometry_aligned_percent);

  procedure set_geometry_bounded
    (window : window_access_t;
     x      : natural;
     y      : natural;
     width  : natural;
     height : natural);
  pragma inline (set_geometry_bounded);

  procedure set_geometry_max (window : window_access_t);
  pragma import (c, set_geometry_max, "AG_WindowSetGeometryMax");

  procedure set_minimum_size
    (window : window_access_t;
     width  : natural;
     height : natural);
  pragma inline (set_minimum_size);

  procedure set_minimum_size_percentage
    (window  : window_access_t;
     percent : percent_t);
  pragma inline (set_minimum_size_percentage);

  procedure maximize (window : window_access_t);
  pragma import (c, maximize, "AG_WindowMaximize");
 
  procedure unmaximize (window : window_access_t);
  pragma import (c, unmaximize, "AG_WindowUnmaximize");
 
  procedure minimize (window : window_access_t);
  pragma import (c, minimize, "AG_WindowMinimize");
 
  procedure unminimize (window : window_access_t);
  pragma import (c, unminimize, "AG_WindowUnminimize");
 
  procedure attach
    (window    : window_access_t;
     subwindow : window_access_t);
  pragma import (c, attach, "AG_WindowAttach");

  procedure detach
    (window    : window_access_t;
     subwindow : window_access_t);
  pragma import (c, detach, "AG_WindowDetach");

  procedure update (window : window_access_t);
  pragma import (c, update, "agar_window_update");

  -- visibility

  procedure show (window : window_access_t);
  pragma import (c, show, "AG_WindowShow");

  procedure hide (window : window_access_t);
  pragma import (c, hide, "AG_WindowHide");

  function is_visible (window : window_access_t) return c.int;
  pragma import (c, is_visible, "agar_window_is_visible");

  function is_visible (window : window_access_t) return boolean;
  pragma inline (is_visible);

  procedure set_visibility
    (window  : window_access_t;
     visible : boolean);
  pragma inline (set_visibility);

  -- focus

  procedure focus (window : window_access_t);
  pragma import (c, focus, "AG_WindowFocus");

  function focus_named (name : string) return boolean;
  pragma inline (focus_named);

  function find_focused (window : window_access_t) return agar.gui.widget.widget_access_t;
  pragma import (c, find_focused, "AG_WidgetFindFocused");

  function is_focused (window : window_access_t) return c.int;
  pragma import (c, is_focused, "agar_window_is_focused");

  function is_focused (window : window_access_t) return boolean;
  pragma inline (is_focused);

  --

  function widget (window : window_access_t) return agar.gui.widget.widget_access_t
    renames agar.gui.types.window_widget;

end agar.gui.window;
