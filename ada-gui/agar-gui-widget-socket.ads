with agar.core.event;
with agar.gui.text;
with agar.gui.widget.icon;

package agar.gui.widget.socket is

  use type c.unsigned;

  type bg_type_t is (SOCKET_PIXMAP, SOCKET_RECT, SOCKET_CIRCLE);
   for bg_type_t use (SOCKET_PIXMAP => 0, SOCKET_RECT => 1, SOCKET_CIRCLE => 2);
   for bg_type_t'size use c.unsigned'size;
  pragma convention (c, bg_type_t);

  type socket_t is limited private;
  type socket_access_t is access all socket_t;
  pragma convention (c, socket_access_t);

  -- API

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
