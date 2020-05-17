with agar.gui.widget.box;

package agar.gui.widget.vbox is

  use type agar.gui.widget.box.flags_t;
  use type c.unsigned;

  type vbox_t is record
    box : aliased agar.gui.widget.box.box_t;
  end record;
  type vbox_access_t is access all vbox_t;
  pragma convention (c, vbox_t);
  pragma convention (c, vbox_access_t);

  type flags_t is new c.unsigned;
  VBOX_HOMOGENOUS : constant flags_t := flags_t (agar.gui.widget.box.BOX_HOMOGENOUS);
  VBOX_HFILL      : constant flags_t := flags_t (agar.gui.widget.box.BOX_HFILL);
  VBOX_VFILL      : constant flags_t := flags_t (agar.gui.widget.box.BOX_VFILL);
  VBOX_EXPAND     : constant flags_t := flags_t (agar.gui.widget.box.BOX_HFILL or agar.gui.widget.box.BOX_VFILL);

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return vbox_access_t;
  pragma import (c, allocate, "agar_gui_widget_vbox_new");

  procedure set_homogenous
    (box        : vbox_access_t;
     homogenous : boolean := true);
  pragma inline (set_homogenous);

  procedure set_padding
    (box     : vbox_access_t;
     padding : natural);
  pragma inline (set_padding);

  procedure set_spacing
    (box     : vbox_access_t;
     spacing : natural);
  pragma inline (set_spacing);

  function widget (box : vbox_access_t) return widget_access_t;
  pragma inline (widget);

end agar.gui.widget.vbox;
