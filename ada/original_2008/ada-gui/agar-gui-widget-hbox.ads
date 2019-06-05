with agar.gui.widget.box;

package agar.gui.widget.hbox is

  use type agar.gui.widget.box.flags_t;
  use type c.unsigned;

  type hbox_t is record
    box : aliased agar.gui.widget.box.box_t;
  end record;
  type hbox_access_t is access all hbox_t;
  pragma convention (c, hbox_t);
  pragma convention (c, hbox_access_t);

  type flags_t is new c.unsigned;
  HBOX_HOMOGENOUS : constant flags_t := flags_t (agar.gui.widget.box.BOX_HOMOGENOUS);
  HBOX_HFILL      : constant flags_t := flags_t (agar.gui.widget.box.BOX_HFILL);
  HBOX_VFILL      : constant flags_t := flags_t (agar.gui.widget.box.BOX_VFILL);
  HBOX_EXPAND     : constant flags_t := flags_t (agar.gui.widget.box.BOX_HFILL or agar.gui.widget.box.BOX_VFILL);

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return hbox_access_t;
  pragma import (c, allocate, "agar_gui_widget_hbox_new");

  procedure set_homogenous
    (box        : hbox_access_t;
     homogenous : boolean := true);
  pragma inline (set_homogenous);

  procedure set_padding
    (box     : hbox_access_t;
     padding : natural);
  pragma inline (set_padding);

  procedure set_spacing
    (box     : hbox_access_t;
     spacing : natural);
  pragma inline (set_spacing);

  function widget (box : hbox_access_t) return widget_access_t;
  pragma inline (widget);

end agar.gui.widget.hbox;
