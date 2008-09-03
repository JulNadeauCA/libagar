with agar.core.types;
with agar.gui.widget.label;
with agar.gui.window;

package agar.gui.widget.icon is

  use type c.unsigned;

  subtype flags_t is c.unsigned;
  ICON_REGEN_LABEL : constant flags_t := 16#01#;
  ICON_DND         : constant flags_t := 16#02#;
  ICON_DBLCLICKED  : constant flags_t := 16#04#;
  ICON_BGFILL      : constant flags_t := 16#08#;

  type name_t is array (1 .. agar.gui.widget.label.max) of aliased c.char;
  pragma convention (c, name_t);

  type icon_t is record
    widget        : widget_t;
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
  type icon_access_t is access all icon_t;
  pragma convention (c, icon_t);
  pragma convention (c, icon_access_t);

end agar.gui.widget.icon;
