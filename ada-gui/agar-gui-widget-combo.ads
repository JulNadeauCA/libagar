with agar.gui.widget.button;
with agar.gui.widget.textbox;
with agar.gui.widget.tlist;
with agar.gui.window;

package agar.gui.widget.combo is

  type combo_t is record
    widget       : widget_t;
    flags        : flags_t;
    textbox      : agar.gui.widget.textbox.textbox_access_t;
    button       : agar.gui.widget.button.button_access_t;
    list         : agar.gui.widget.tlist.tlist_access_t;
    panel        : agar.gui.window.window_access_t;
    width_saved  : c.int;
    height_saved : c.int;
    width_pre    : c.int;
    height_pre   : c.int;
  end record;
  type combo_access_t is access all combo_t;
  pragma convention (c, combo_t);
  pragma convention (c, combo_access_t);

end agar.gui.widget.combo;
