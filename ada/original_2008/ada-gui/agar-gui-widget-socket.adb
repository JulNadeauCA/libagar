package body agar.gui.widget.socket is

  package cbinds is
    function from_bitmap
      (parent : widget_access_t;
       flags  : flags_t;
       file   : cs.chars_ptr) return socket_access_t;
    pragma import (c, from_bitmap, "AG_SocketFromBMP");

    procedure set_padding
      (socket : socket_access_t;
       left   : c.int;
       right  : c.int;
       top    : c.int;
       bottom : c.int);
    pragma import (c, set_padding, "AG_SocketSetPadding");

    procedure shape_rectangle
      (socket : socket_access_t;
       width  : c.unsigned;
       height : c.unsigned);
    pragma import (c, shape_rectangle, "AG_SocketBgRect");

    procedure shape_circle
      (socket : socket_access_t;
       radius : c.unsigned);
    pragma import (c, shape_circle, "AG_SocketBgCircle");
  end cbinds;

  function from_bitmap
    (parent : widget_access_t;
     flags  : flags_t;
     file   : string) return socket_access_t
  is
    ca_file : aliased c.char_array := c.to_c (file);
  begin
    return cbinds.from_bitmap
      (parent => parent,
       flags  => flags,
       file   => cs.to_chars_ptr (ca_file'unchecked_access));
  end from_bitmap;

  procedure set_padding
    (socket : socket_access_t;
     left   : natural;
     right  : natural;
     top    : natural;
     bottom : natural) is
  begin
    cbinds.set_padding
      (socket => socket,
       left   => c.int (left),
       right  => c.int (right),
       top    => c.int (top),
       bottom => c.int (bottom));
  end set_padding;

  procedure shape_rectangle
    (socket : socket_access_t;
     width  : natural;
     height : natural) is
  begin
    cbinds.shape_rectangle
      (socket => socket,
       width  => c.unsigned (width),
       height => c.unsigned (height));
  end shape_rectangle;

  procedure shape_circle
    (socket : socket_access_t;
     radius : natural) is
  begin
    cbinds.shape_circle
      (socket => socket,
       radius => c.unsigned (radius));
  end shape_circle;

  function widget (socket : socket_access_t) return widget_access_t is
  begin
    return socket.widget'access;
  end widget;

  function icon (socket : socket_access_t) return agar.gui.widget.icon.icon_access_t is
  begin
    return socket.icon;
  end icon;

end agar.gui.widget.socket;
