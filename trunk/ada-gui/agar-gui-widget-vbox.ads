with agar.gui.widget.box;

package agar.gui.widget.vbox is

  use type c.unsigned;

  type vbox_t is record
    box : agar.gui.widget.box.box_t;
  end record;

  subtype flags_t is c.unsigned;
  VBOX_HOMOGENOUS : constant flags_t := agar.gui.widget.box.BOX_HOMOGENOUS;
  VBOX_HFILL      : constant flags_t := agar.gui.widget.box.BOX_HFILL;
  VBOX_VFILL      : constant flags_t := agar.gui.widget.box.BOX_VFILL;
  VBOX_EXPAND     : constant flags_t := agar.gui.widget.box.BOX_HFILL or agar.gui.widget.box.BOX_VFILL;

end agar.gui.widget.vbox;
