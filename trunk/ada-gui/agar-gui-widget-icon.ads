with agar.core.types;
with agar.gui.surface;
with agar.gui.widget.label;
with agar.gui.window;

package agar.gui.widget.icon is

  use type c.unsigned;

  subtype flags_t is c.unsigned;
  ICON_REGEN_LABEL : constant flags_t := 16#01#;
  ICON_DND         : constant flags_t := 16#02#;
  ICON_DBLCLICKED  : constant flags_t := 16#04#;
  ICON_BGFILL      : constant flags_t := 16#08#;

  type icon_t is limited private;
  type icon_access_t is access all icon_t;
  pragma convention (c, icon_access_t);

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return icon_access_t;
  pragma import (c, allocate, "AG_IconNew");

  function allocate_from_surface
    (surface : agar.gui.surface.surface_access_t) return icon_access_t;
  pragma import (c, allocate_from_surface, "AG_IconFromSurface");

  function allocate_from_bitmap
    (filename : string) return icon_access_t;
  pragma inline (allocate_from_bitmap);

--  procedure set_padding
--    (icon   : icon_access_t;
--     left   : natural;
--     right  : natural;
--     top    : natural;
--     bottom : natural);
--  pragma inline (set_padding);

  procedure set_surface
    (icon    : icon_access_t;
     surface : agar.gui.surface.surface_access_t);
  pragma import (c, set_surface, "AG_IconSetSurface");

  procedure set_surface_no_copy
    (icon    : icon_access_t;
     surface : agar.gui.surface.surface_access_t);
  pragma import (c, set_surface_no_copy, "AG_IconSetSurfaceNODUP");

  procedure set_text
    (icon : icon_access_t;
     text : string);
  pragma inline (set_text);

  procedure set_background_fill
    (icon  : icon_access_t;
     fill  : boolean;
     color : agar.core.types.uint32_t);
  pragma inline (set_background_fill);

  function widget (icon : icon_access_t) return widget_access_t;
  pragma inline (widget);

private

  type name_t is array (1 .. agar.gui.widget.label.max) of aliased c.char;
  pragma convention (c, name_t);

  type icon_t is record
    widget        : aliased widget_t;
    flags         : flags_t;
    surface       : c.int;
    label_text    : name_t;
    label_surface : c.int;
    label_pad     : c.int;
    window        : agar.gui.window.window_access_t;
    socket        : agar.core.types.void_ptr_t;
    x_saved       : c.int;
    y_saved       : c.int;
    w_saved       : c.int;
    h_saved       : c.int;
    c_background  : agar.core.types.uint32_t;
  end record;
  pragma convention (c, icon_t);

end agar.gui.widget.icon;
