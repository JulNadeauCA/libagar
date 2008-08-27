with agar.core.tail_queue;
with agar.core.types;
with agar.gui.widget;

package agar.gui.window is

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

  --
  -- API
  --

  function find_focused (window : window_access_t) return agar.gui.widget.widget_access_t;
  pragma import (c, find_focused, "AG_WidgetFindFocused");

end agar.gui.window;
