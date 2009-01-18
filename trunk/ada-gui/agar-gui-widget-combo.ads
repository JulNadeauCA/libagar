with agar.gui.widget.button;
with agar.gui.widget.textbox;
with agar.gui.widget.tlist;
with agar.gui.window;

package agar.gui.widget.combo is

  use type c.unsigned;

  type flags_t is new c.unsigned;
  COMBO_POLL     : constant flags_t := 16#01#;
  COMBO_TREE     : constant flags_t := 16#02#;
  COMBO_ANY_TEXT : constant flags_t := 16#04#;
  COMBO_HFILL    : constant flags_t := 16#08#;
  COMBO_VFILL    : constant flags_t := 16#10#;
  COMBO_EXPAND   : constant flags_t := COMBO_HFILL or COMBO_VFILL;

  type combo_t is limited private;
  type combo_access_t is access all combo_t;
  pragma convention (c, combo_access_t);

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string) return combo_access_t;
  pragma inline (allocate);

  procedure size_hint
    (combo  : combo_access_t;
     text   : string;
     items  : integer);
  pragma inline (size_hint);

  procedure size_hint_pixels
    (combo  : combo_access_t;
     width  : positive;
     height : positive);
  pragma inline (size_hint_pixels);

  procedure select_item
    (combo  : combo_access_t;
     item   : agar.gui.widget.tlist.tlist_access_t);
  pragma import (c, select_item, "AG_ComboSelect");

  procedure select_pointer
    (combo  : combo_access_t;
     ptr    : agar.core.types.void_ptr_t);
  pragma import (c, select_pointer, "AG_ComboSelectPointer");

  procedure select_text
    (combo  : combo_access_t;
     text   : string);
  pragma inline (select_text);

private

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
  pragma convention (c, combo_t);

end agar.gui.widget.combo;
