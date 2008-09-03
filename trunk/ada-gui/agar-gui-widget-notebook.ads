with agar.core.tail_queue;
with agar.gui.text;
with agar.gui.widget.box;

package agar.gui.widget.notebook is

  use type c.unsigned;

  type tab_t;
  type tab_access_t is access all tab_t;
  pragma convention (c, tab_access_t);

  package tab_tail_queue is new agar.core.tail_queue
    (entry_type => tab_access_t);

  type tab_alignment_t is (
    NOTEBOOK_TABS_TOP,
    NOTEBOOK_TABS_BOTTOM,
    NOTEBOOK_TABS_LEFT,
    NOTEBOOK_TABS_RIGHT
  );
  for tab_alignment_t use (
    NOTEBOOK_TABS_TOP    => 0,
    NOTEBOOK_TABS_BOTTOM => 1,
    NOTEBOOK_TABS_LEFT   => 2,
    NOTEBOOK_TABS_RIGHT  => 3
  );
  for tab_alignment_t'size use c.unsigned'size;
  pragma convention (c, tab_alignment_t);

  label_max : constant := 64;

  type text_t is array (1 .. label_max) of aliased c.char;
  pragma convention (c, text_t);

  type tab_t is record
    box        : agar.gui.widget.box.box_t;
    label      : c.int;
    label_text : text_t;
    tabs       : tab_tail_queue.entry_t;
  end record;
  pragma convention (c, tab_t);

  subtype flags_t is c.unsigned;
  NOTEBOOK_HFILL     : constant flags_t := 16#01#;
  NOTEBOOK_VFILL     : constant flags_t := 16#02#;
  NOTEBOOK_HIDE_TABS : constant flags_t := 16#04#;
  NOTEBOOK_EXPAND    : constant flags_t := NOTEBOOK_HFILL or NOTEBOOK_VFILL;

  type notebook_t is record
    widget : widget_t;
    tab_align           : tab_alignment_t;
    flags               : flags_t;
    bar_w               : c.int;
    bar_h               : c.int;
    cont_w              : c.int;
    cont_h              : c.int;
    spacing             : c.int;
    padding             : c.int;
    tab_font            : agar.gui.text.font_access_t;
    label_partial       : c.int;
    label_partial_width : c.int;
    sel_tab             : tab_access_t;
    tabs                : tab_tail_queue.head_t;
  end record;
  type notebook_access_t is access all notebook_t;
  pragma convention (c, notebook_t);
  pragma convention (c, notebook_access_t);

end agar.gui.widget.notebook;
