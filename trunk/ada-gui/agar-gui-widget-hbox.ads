with agar.gui.widget.box;

package agar.gui.widget.hbox is

  use type c.unsigned;

  type hbox_t is record
    box : agar.gui.widget.box.box_t;
  end record;

  subtype flags_t is c.unsigned;
  HBOX_HOMOGENOUS : constant flags_t := agar.gui.widget.box.BOX_HOMOGENOUS;
  HBOX_HFILL      : constant flags_t := agar.gui.widget.box.BOX_HFILL;
  HBOX_VFILL      : constant flags_t := agar.gui.widget.box.BOX_VFILL;
  HBOX_EXPAND     : constant flags_t := agar.gui.widget.box.BOX_HFILL or agar.gui.widget.box.BOX_VFILL;

end agar.gui.widget.hbox;
