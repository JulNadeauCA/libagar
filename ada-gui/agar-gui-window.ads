with agar.core.tail_queue;
with agar.core.types;
with agar.gui.surface;
with agar.gui.widget;
with interfaces.c.strings;

package agar.gui.window is
  package cs renames interfaces.c.strings;

  use type c.unsigned;

  --
  -- forward declarations
  --
  type window_t;
  type window_access_t is access all window_t;
  pragma convention (c, window_access_t);

  package window_tail_queue is new agar.core.tail_queue
    (entry_type => window_access_t);

  --
  -- constants
  --

  caption_max : constant c.unsigned := 512;

  --
  -- types
  --

  subtype flags_t is c.unsigned;
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

  type alignment_t is (
    WINDOW_TL,
    WINDOW_TC,
    WINDOW_TR,
    WINDOW_ML,
    WINDOW_MC,
    WINDOW_MR,
    WINDOW_BL,
    WINDOW_BC,
    WINDOW_BR
  );
  for alignment_t use (
    WINDOW_TL => 0,
    WINDOW_TC => 1,
    WINDOW_TR => 2,
    WINDOW_ML => 3,
    WINDOW_MC => 4,
    WINDOW_MR => 5,
    WINDOW_BL => 6,
    WINDOW_BC => 7,
    WINDOW_BR => 8
  );
  for alignment_t'size use c.unsigned'size;
  pragma convention (c, alignment_t);

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

  type window_caption_t is array (1 .. caption_max) of aliased c.char;
  pragma convention (c, window_caption_t);

  type window_t is record
    widget    : agar.gui.widget.widget_t;
    flags     : flags_t;
    caption   : window_caption_t;
    visible   : c.int;
    tbar      : agar.core.types.void_ptr_t; -- XXX: titlebar_access_t
    alignment : alignment_t;
    spacing   : c.int;
    tpad      : c.int;
    bpad      : c.int;
    lpad      : c.int;
    rpad      : c.int;
    minw      : c.int;
    minh      : c.int;
    savx      : c.int;
    savy      : c.int;
    savw      : c.int;
    savh      : c.int;
    subwins   : window_tail_queue.head_t;
    windows   : window_tail_queue.entry_t;
    swins     : window_tail_queue.entry_t;
    detach    : window_tail_queue.entry_t;
    icon      : agar.core.types.void_ptr_t; -- XXX: icon_access_t
  end record;

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
     left   : c.int;
     right  : c.int;
     top    : c.int;
     bottom : c.int);
  pragma import (c, set_padding, "AG_WindowSetPadding");

  procedure set_padding
    (window : window_access_t;
     left   : natural;
     right  : natural;
     top    : natural;
     bottom : natural);
  pragma inline (set_padding);

  procedure set_spacing
    (window  : window_access_t;
     spacing : c.int);
  pragma import (c, set_spacing, "AG_WindowSetSpacing");

  procedure set_spacing
    (window  : window_access_t;
     spacing : natural);
  pragma inline (set_spacing);
 
  procedure set_position
    (window    : window_access_t;
     alignment : alignment_t;
     cascade   : c.int);
  pragma import (c, set_position, "AG_WindowSetPosition");

  procedure set_position
    (window    : window_access_t;
     alignment : alignment_t;
     cascade   : boolean);
  pragma inline (set_position);

  procedure set_geometry
    (window : window_access_t;
     x      : c.int;
     y      : c.int;
     width  : c.int;
     height : c.int);
  pragma import (c, set_geometry, "agar_window_set_geometry");

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
     width     : c.int;
     height    : c.int);
  pragma import (c, set_geometry_aligned, "agar_window_set_geometry_aligned");

  procedure set_geometry_aligned
    (window    : window_access_t;
     alignment : alignment_t;
     width     : positive;
     height    : positive);
  pragma inline (set_geometry_aligned);
 
  procedure set_geometry_aligned_percent
    (window    : window_access_t;
     alignment : alignment_t;
     width     : c.int;
     height    : c.int);
  pragma import (c, set_geometry_aligned_percent, "agar_window_set_geometry_aligned_percent");

  procedure set_geometry_aligned_percent
    (window    : window_access_t;
     alignment : alignment_t;
     width     : percent_t;
     height    : percent_t);
  pragma inline (set_geometry_aligned_percent);

  procedure set_geometry_bounded
    (window : window_access_t;
     x      : c.int;
     y      : c.int;
     width  : c.int;
     height : c.int);
  pragma import (c, set_geometry_bounded, "agar_window_set_geometry_bounded");

  procedure set_geometry_bounded
    (window : window_access_t;
     x      : natural;
     y      : natural;
     width  : natural;
     height : natural);
  pragma inline (set_geometry_bounded);

  procedure set_geometry_max (window : window_access_t);
  pragma import (c, set_geometry_max, "AG_WindowSetGeometryMax");

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
     visible : c.int);
  pragma import (c, set_visibility, "AG_WindowSetVisibility");

  procedure set_visibility
    (window  : window_access_t;
     visible : boolean);
  pragma inline (set_visibility);

  -- focus

  procedure focus (window : window_access_t);
  pragma import (c, focus, "AG_WindowFocus");

  function focus_named (name : cs.chars_ptr) return c.int;
  pragma import (c, focus_named, "AG_WindowFocusNamed");

  function focus_named (name : string) return boolean;
  pragma inline (focus_named);

  function find_focused (window : window_access_t) return agar.gui.widget.widget_access_t;
  pragma import (c, find_focused, "AG_WidgetFindFocused");

  function is_focused (window : window_access_t) return c.int;
  pragma import (c, is_focused, "agar_window_is_focused");

  function is_focused (window : window_access_t) return boolean;
  pragma inline (is_focused);

end agar.gui.window;
