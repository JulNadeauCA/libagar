with agar.core.event;
with agar.gui.text;
with agar.gui.widget.icon;

package agar.gui.widget.socket is

  use type c.unsigned;

  type bg_type_t is (SOCKET_PIXMAP, SOCKET_RECT, SOCKET_CIRCLE);
   for bg_type_t use (SOCKET_PIXMAP => 0, SOCKET_RECT => 1, SOCKET_CIRCLE => 2);
   for bg_type_t'size use c.unsigned'size;
  pragma convention (c, bg_type_t);

  type flags_t is new c.unsigned;
  SOCKET_HFILL     : constant flags_t := 16#01#;
  SOCKET_VFILL     : constant flags_t := 16#02#;
  SOCKET_EXPAND    : constant flags_t := SOCKET_HFILL or SOCKET_VFILL;
  SOCKET_MOUSEOVER : constant flags_t := 16#04#;
  STICKY_STATE     : constant flags_t := 16#08#;

  type socket_t is limited private;
  type socket_access_t is access all socket_t;
  pragma convention (c, socket_access_t);

  type insert_callback_t is access function
    (socket : socket_access_t;
     icon   : agar.gui.widget.icon.icon_access_t) return c.int;
  pragma convention (c, insert_callback_t);

  type remove_callback_t is access procedure
    (socket : socket_access_t;
     icon   : agar.gui.widget.icon.icon_access_t);
  pragma convention (c, remove_callback_t);

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return socket_access_t;
  pragma import (c, allocate, "AG_SocketNew");

  function from_surface
    (parent  : widget_access_t;
     flags   : flags_t;
     surface : agar.gui.surface.surface_access_t) return socket_access_t;
  pragma import (c, from_surface, "AG_SocketFromSurface");

  function from_bitmap
    (parent : widget_access_t;
     flags  : flags_t;
     file   : string) return socket_access_t;
  pragma inline (from_bitmap);

  procedure set_insert_callback
    (socket   : socket_access_t;
     callback : insert_callback_t);
  pragma import (c, set_insert_callback, "AG_SocketInsertFn");

  procedure set_remove_callback
    (socket   : socket_access_t;
     callback : remove_callback_t);
  pragma import (c, set_remove_callback, "AG_SocketRemoveFn");

  procedure set_padding
    (socket : socket_access_t;
     left   : natural;
     right  : natural;
     top    : natural;
     bottom : natural);
  pragma inline (set_padding);

  procedure shape_rectangle
    (socket : socket_access_t;
     width  : natural;
     height : natural);
  pragma inline (shape_rectangle);

  procedure shape_circle
    (socket : socket_access_t;
     radius : natural);
  pragma inline (shape_circle);

  procedure shape_pixmap
    (socket  : socket_access_t;
     surface : agar.gui.surface.surface_access_t);
  pragma import (c, shape_pixmap, "AG_SocketBgPixmap");

  procedure shape_pixmap_no_copy
    (socket  : socket_access_t;
     surface : agar.gui.surface.surface_access_t);
  pragma import (c, shape_pixmap_no_copy, "AG_SocketBgPixmapNODUP");

  function widget (socket : socket_access_t) return widget_access_t;
  pragma inline (widget);

  function icon (socket : socket_access_t) return agar.gui.widget.icon.icon_access_t;
  pragma inline (icon);

private

  type pixmap_t is record
    s : c.int;
  end record;
  pragma convention (c, pixmap_t);

  type rect_t is record
    w : c.int;
    h : c.int;
  end record;
  pragma convention (c, rect_t);

  type circle_t is record
    r : c.int;
  end record;
  pragma convention (c, circle_t);

  type bg_data_selector_t is (DATA_PIXMAP, DATA_RECT, DATA_CIRCLE);
  type bg_data_t (member : bg_data_selector_t := DATA_PIXMAP) is record
    case member is
      when DATA_PIXMAP => pixmap : pixmap_t;
      when DATA_RECT   => rect   : rect_t;
      when DATA_CIRCLE => circle : circle_t;
    end case;
  end record;
  pragma convention (c, bg_data_t);
  pragma unchecked_union (bg_data_t);

  type socket_t is record
    widget     : aliased widget_t;
    state      : c.int;
    count      : c.int;
    flags      : flags_t;
    bg_type    : bg_type_t;
    bg_data    : bg_data_t;
    justify    : agar.gui.text.justify_t;
    pad_left   : c.int;
    pad_right  : c.int;
    pad_top    : c.int;
    pad_bottom : c.int;
    icon       : agar.gui.widget.icon.icon_access_t;
    insert_fn  : access function
      (sock : socket_access_t;
       icon : agar.gui.widget.icon.icon_access_t) return c.int;
    remove_fn  : access procedure
      (sock : socket_access_t;
       icon : agar.gui.widget.icon.icon_access_t);
    overlay_fn : agar.core.event.event_access_t;
  end record;
  pragma convention (c, socket_t);

end agar.gui.widget.socket;
