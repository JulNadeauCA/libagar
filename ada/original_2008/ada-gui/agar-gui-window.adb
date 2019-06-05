with interfaces.c.strings;

package body agar.gui.window is
  package cs renames interfaces.c.strings;

  use type c.int;

  function allocate_named
    (flags : flags_t := 0;
     name  : cs.chars_ptr) return window_access_t;
  pragma import (c, allocate_named, "AG_WindowNewNamedS");

  function allocate_named
    (flags : flags_t := 0;
     name  : string) return window_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return allocate_named
      (flags => flags,
       name  => cs.to_chars_ptr (ca_name'unchecked_access));
  end allocate_named;

  procedure set_caption
    (window  : window_access_t;
     caption : cs.chars_ptr);
  pragma import (c, set_caption, "AG_WindowSetCaptionS");

  procedure set_caption
    (window  : window_access_t;
     caption : string)
  is
    ca_caption : aliased c.char_array := c.to_c (caption);
  begin
    set_caption
      (window  => window,
       caption => cs.to_chars_ptr (ca_caption'unchecked_access));
  end set_caption;

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
     bottom : natural) is
  begin
    set_padding
      (window => window,
       left   => c.int (left),
       right  => c.int (right),
       top    => c.int (top),
       bottom => c.int (bottom));
  end set_padding;

  procedure set_spacing
    (window  : window_access_t;
     spacing : c.int);
  pragma import (c, set_spacing, "AG_WindowSetSpacing");

  procedure set_spacing
    (window  : window_access_t;
     spacing : natural) is
  begin
    set_spacing (window, c.int (spacing));
  end set_spacing;

  procedure set_position
    (window    : window_access_t;
     alignment : alignment_t;
     cascade   : c.int);
  pragma import (c, set_position, "AG_WindowSetPosition");

  procedure set_position
    (window    : window_access_t;
     alignment : alignment_t;
     cascade   : boolean) is
  begin
    if cascade then
      set_position (window, alignment, 1);
    else
      set_position (window, alignment, 0);
    end if;
  end set_position;

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
     height : natural) is
  begin
    set_geometry
      (window => window,
       x      => c.int (x),
       y      => c.int (y),
       width  => c.int (width),
       height => c.int (height));
  end set_geometry;

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
     height    : positive) is
  begin
    set_geometry_aligned
      (window    => window,
       alignment => alignment,
       width     => c.int (width),
       height    => c.int (height));
  end set_geometry_aligned;

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
     height    : percent_t) is
  begin
    set_geometry_aligned_percent
      (window    => window,
       alignment => alignment,
       width     => c.int (width),
       height    => c.int (height));
  end set_geometry_aligned_percent;

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
     height : natural) is
  begin
    set_geometry_bounded
      (window => window,
       x      => c.int (x),
       y      => c.int (y),
       width  => c.int (width),
       height => c.int (height));
  end set_geometry_bounded;

  procedure set_minimum_size
    (window : window_access_t;
     width  : c.int;
     height : c.int);
  pragma import (c, set_minimum_size, "AG_WindowSetMinSize");
 
  procedure set_minimum_size
    (window : window_access_t;
     width  : natural;
     height : natural) is
  begin
    set_minimum_size
      (window => window,
       width  => c.int (width),
       height => c.int (height));
  end set_minimum_size;
 
  procedure set_minimum_size_percentage
    (window  : window_access_t;
     percent : c.int);
  pragma import (c, set_minimum_size_percentage, "AG_WindowSetMinSizePct");
                
  procedure set_minimum_size_percentage
    (window  : window_access_t;
     percent : percent_t) is
  begin
    set_minimum_size_percentage
      (window  => window,
       percent => c.int (percent));
  end set_minimum_size_percentage;

  function is_visible (window : window_access_t) return boolean is
  begin
    return is_visible (window) = 1;
  end is_visible;

  procedure set_visibility
    (window  : window_access_t;
     visible : c.int);
  pragma import (c, set_visibility, "AG_WindowSetVisibility");

  procedure set_visibility
    (window  : window_access_t;
     visible : boolean) is
  begin
    if visible then
      set_visibility (window, 1);
    else
      set_visibility (window, 0);
    end if;
  end set_visibility;

  function focus_named (name : cs.chars_ptr) return c.int;
  pragma import (c, focus_named, "AG_WindowFocusNamed");

  function focus_named (name : string) return boolean is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return focus_named (cs.to_chars_ptr (ca_name'unchecked_access)) = 0;
  end focus_named;

  function is_focused (window : window_access_t) return boolean is
  begin
    return is_focused (window) = 1;
  end is_focused;

end agar.gui.window;
