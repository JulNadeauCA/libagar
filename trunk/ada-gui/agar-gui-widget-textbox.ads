with agar.gui.widget.scrollbar;
with agar.gui.widget.editable;

package agar.gui.widget.textbox is

  use type c.unsigned;

  type flags_t is new c.unsigned;
  TEXTBOX_MULTILINE     : constant flags_t := 16#00001#;
  TEXTBOX_PASSWORD      : constant flags_t := 16#00004#;
  TEXTBOX_ABANDON_FOCUS : constant flags_t := 16#00008#;
  TEXTBOX_COMBO         : constant flags_t := 16#00010#;
  TEXTBOX_HFILL         : constant flags_t := 16#00020#;
  TEXTBOX_VFILL         : constant flags_t := 16#00040#;
  TEXTBOX_EXPAND        : constant flags_t := TEXTBOX_HFILL or TEXTBOX_VFILL;
  TEXTBOX_READONLY      : constant flags_t := 16#00100#;
  TEXTBOX_INT_ONLY      : constant flags_t := 16#00200#;
  TEXTBOX_FLT_ONLY      : constant flags_t := 16#00400#;
  TEXTBOX_CATCH_TAB     : constant flags_t := 16#00800#;
  TEXTBOX_CURSOR_MOVING : constant flags_t := 16#01000#;
  TEXTBOX_NO_HFILL      : constant flags_t := 16#02000#;
  TEXTBOX_STATIC        : constant flags_t := 16#04000#;
  TEXTBOX_NOEMACS       : constant flags_t := 16#08000#;
  TEXTBOX_NOWORDSEEK    : constant flags_t := 16#10000#;
  TEXTBOX_NOLATIN1      : constant flags_t := 16#20000#;

  type textbox_t is record
    widget          : widget_t;
    editable        : agar.gui.widget.editable.editable_access_t;
    label_text      : cs.chars_ptr;
    label           : c.int;
    flags           : flags_t;
    box_pad_x       : c.int;
    box_pad_y       : c.int;
    label_pad_left  : c.int;
    label_pad_right : c.int;
    width_label     : c.int;
    height_label    : c.int;
    horiz_scrollbar : agar.gui.widget.scrollbar.scrollbar_access_t;
    vert_scrollbar  : agar.gui.widget.scrollbar.scrollbar_access_t;
  end record;
  type textbox_access_t is access all textbox_t;
  pragma convention (c, textbox_t);
  pragma convention (c, textbox_access_t);

  string_max : constant := agar.gui.widget.editable.string_max;

end agar.gui.widget.textbox;
