with agar.core.types;

package agar.gui.colors is

  type color_selection_t is (
    BG_COLOR,
    FRAME_COLOR,
    LINE_COLOR,
    TEXT_COLOR,
    WINDOW_BG_COLOR,
    WINDOW_HI_COLOR,
    WINDOW_LO_COLOR,
    TITLEBAR_FOCUSED_COLOR,
    TITLEBAR_UNFOCUSED_COLOR,
    TITLEBAR_CAPTION_COLOR,
    BUTTON_COLOR,
    BUTTON_TXT_COLOR,
    DISABLED_COLOR,
    CHECKBOX_COLOR,
    CHECKBOX_TXT_COLOR,
    GRAPH_BG_COLOR,
    GRAPH_XAXIS_COLOR,
    HSVPAL_CIRCLE_COLOR,
    HSVPAL_TILE1_COLOR,
    HSVPAL_TILE2_COLOR,
    MENU_UNSEL_COLOR,
    MENU_SEL_COLOR,
    MENU_OPTION_COLOR,
    MENU_TXT_COLOR,
    MENU_SEP1_COLOR,
    MENU_SEP2_COLOR,
    NOTEBOOK_BG_COLOR,
    NOTEBOOK_SEL_COLOR,
    NOTEBOOK_TXT_COLOR,
    RADIO_SEL_COLOR,
    RADIO_OVER_COLOR,
    RADIO_HI_COLOR,
    RADIO_LO_COLOR,
    RADIO_TXT_COLOR,
    SCROLLBAR_COLOR,
    SCROLLBAR_BTN_COLOR,
    SCROLLBAR_ARR1_COLOR,
    SCROLLBAR_ARR2_COLOR,
    SEPARATOR_LINE1_COLOR,
    SEPARATOR_LINE2_COLOR,
    TABLEVIEW_COLOR,
    TABLEVIEW_HEAD_COLOR,
    TABLEVIEW_HTXT_COLOR,
    TABLEVIEW_CTXT_COLOR,
    TABLEVIEW_LINE_COLOR,
    TABLEVIEW_SEL_COLOR,
    TEXTBOX_COLOR,
    TEXTBOX_TXT_COLOR,
    TEXTBOX_CURSOR_COLOR,
    TLIST_TXT_COLOR,
    TLIST_BG_COLOR,
    TLIST_LINE_COLOR,
    TLIST_SEL_COLOR,
    MAPVIEW_GRID_COLOR,
    MAPVIEW_CURSOR_COLOR,
    MAPVIEW_TILE1_COLOR,
    MAPVIEW_TILE2_COLOR,
    MAPVIEW_MSEL_COLOR,
    MAPVIEW_ESEL_COLOR,
    TILEVIEW_TILE1_COLOR,
    TILEVIEW_TILE2_COLOR,
    TILEVIEW_TEXTBG_COLOR,
    TILEVIEW_TEXT_COLOR,
    TRANSPARENT_COLOR,
    HSVPAL_BAR1_COLOR,
    HSVPAL_BAR2_COLOR,
    PANE_COLOR,
    PANE_CIRCLE_COLOR,
    MAPVIEW_RSEL_COLOR,
    MAPVIEW_ORIGIN_COLOR,
    FOCUS_COLOR,
    TABLE_COLOR,
    TABLE_LINE_COLOR,
    FIXED_BG_COLOR,
    FIXED_BOX_COLOR,
    TEXT_DISABLED_COLOR,
    MENU_TXT_DISABLED_COLOR,
    SOCKET_COLOR,
    SOCKET_LABEL_COLOR,
    SOCKET_HIGHLIGHT_COLOR,
    PROGRESS_BAR_COLOR,
    LAST_COLOR
  );

  for color_selection_t use (
    BG_COLOR                 => 0,
    FRAME_COLOR              => 1,
    LINE_COLOR               => 2,
    TEXT_COLOR               => 3,
    WINDOW_BG_COLOR          => 4,
    WINDOW_HI_COLOR          => 5,
    WINDOW_LO_COLOR          => 6,
    TITLEBAR_FOCUSED_COLOR   => 7,
    TITLEBAR_UNFOCUSED_COLOR => 8,
    TITLEBAR_CAPTION_COLOR   => 9,
    BUTTON_COLOR             => 10,
    BUTTON_TXT_COLOR         => 11,
    DISABLED_COLOR           => 12,
    CHECKBOX_COLOR           => 13,
    CHECKBOX_TXT_COLOR       => 14,
    GRAPH_BG_COLOR           => 15,
    GRAPH_XAXIS_COLOR        => 16,
    HSVPAL_CIRCLE_COLOR      => 17,
    HSVPAL_TILE1_COLOR       => 18,
    HSVPAL_TILE2_COLOR       => 19,
    MENU_UNSEL_COLOR         => 20,
    MENU_SEL_COLOR           => 21,
    MENU_OPTION_COLOR        => 22,
    MENU_TXT_COLOR           => 23,
    MENU_SEP1_COLOR          => 24,
    MENU_SEP2_COLOR          => 25,
    NOTEBOOK_BG_COLOR        => 26,
    NOTEBOOK_SEL_COLOR       => 27,
    NOTEBOOK_TXT_COLOR       => 28,
    RADIO_SEL_COLOR          => 29,
    RADIO_OVER_COLOR         => 30,
    RADIO_HI_COLOR           => 31,
    RADIO_LO_COLOR           => 32,
    RADIO_TXT_COLOR          => 33,
    SCROLLBAR_COLOR          => 34,
    SCROLLBAR_BTN_COLOR      => 35,
    SCROLLBAR_ARR1_COLOR     => 36,
    SCROLLBAR_ARR2_COLOR     => 37,
    SEPARATOR_LINE1_COLOR    => 38,
    SEPARATOR_LINE2_COLOR    => 39,
    TABLEVIEW_COLOR          => 40,
    TABLEVIEW_HEAD_COLOR     => 41,
    TABLEVIEW_HTXT_COLOR     => 42,
    TABLEVIEW_CTXT_COLOR     => 43,
    TABLEVIEW_LINE_COLOR     => 44,
    TABLEVIEW_SEL_COLOR      => 45,
    TEXTBOX_COLOR            => 46,
    TEXTBOX_TXT_COLOR        => 47,
    TEXTBOX_CURSOR_COLOR     => 48,
    TLIST_TXT_COLOR          => 49,
    TLIST_BG_COLOR           => 50,
    TLIST_LINE_COLOR         => 51,
    TLIST_SEL_COLOR          => 52,
    MAPVIEW_GRID_COLOR       => 53,
    MAPVIEW_CURSOR_COLOR     => 54,
    MAPVIEW_TILE1_COLOR      => 55,
    MAPVIEW_TILE2_COLOR      => 56,
    MAPVIEW_MSEL_COLOR       => 57,
    MAPVIEW_ESEL_COLOR       => 58,
    TILEVIEW_TILE1_COLOR     => 59,
    TILEVIEW_TILE2_COLOR     => 60,
    TILEVIEW_TEXTBG_COLOR    => 61,
    TILEVIEW_TEXT_COLOR      => 62,
    TRANSPARENT_COLOR        => 63,
    HSVPAL_BAR1_COLOR        => 64,
    HSVPAL_BAR2_COLOR        => 65,
    PANE_COLOR               => 66,
    PANE_CIRCLE_COLOR        => 67,
    MAPVIEW_RSEL_COLOR       => 68,
    MAPVIEW_ORIGIN_COLOR     => 69,
    FOCUS_COLOR              => 70,
    TABLE_COLOR              => 71,
    TABLE_LINE_COLOR         => 72,
    FIXED_BG_COLOR           => 73,
    FIXED_BOX_COLOR          => 74,
    TEXT_DISABLED_COLOR      => 75,
    MENU_TXT_DISABLED_COLOR  => 76,
    SOCKET_COLOR             => 77,
    SOCKET_LABEL_COLOR       => 78,
    SOCKET_HIGHLIGHT_COLOR   => 79,
    PROGRESS_BAR_COLOR       => 80,
    LAST_COLOR               => 81
  );
  for color_selection_t'size use c.unsigned'size;
  pragma convention (c, color_selection_t);

  type color_t is record
    r : agar.core.types.uint8_t;
    g : agar.core.types.uint8_t;
    b : agar.core.types.uint8_t;
    a : agar.core.types.uint8_t;
  end record;
  type color_access_t is access all color_t;
  pragma convention (c, color_t);
  pragma convention (c, color_access_t);

  type blend_func_t is (
    ALPHA_OVERLAY,
    ALPHA_SRC,
    ALPHA_DST,
    ALPHA_ONE_MINUS_DST,
    ALPHA_ONE_MINUS_SRC
  );
  for blend_func_t use (
    ALPHA_OVERLAY       => 0,
    ALPHA_SRC           => 1,
    ALPHA_DST           => 2,
    ALPHA_ONE_MINUS_DST => 3,
    ALPHA_ONE_MINUS_SRC => 4
  );
  for blend_func_t'size use c.unsigned'size;
  pragma convention (c, blend_func_t);

  procedure init;
  pragma import (c, init, "AG_ColorsInit");

  procedure destroy;
  pragma import (c, destroy, "AG_ColorsDestroy");

  function load (path : string) return boolean;
  pragma inline (load);

  function save (path : string) return boolean;
  pragma inline (save);

  function save_default return boolean;
  pragma inline (save_default);

end agar.gui.colors;
