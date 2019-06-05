with agar.gui.types;

package agar.gui.widget.titlebar is

  subtype titlebar_t is agar.gui.types.widget_titlebar_t;
  subtype titlebar_access_t is agar.gui.types.widget_titlebar_access_t;

  subtype flags_t is agar.gui.types.widget_titlebar_flags_t;
  TITLEBAR_NO_CLOSE    : constant flags_t := 16#01#;
  TITLEBAR_NO_MINIMIZE : constant flags_t := 16#02#;
  TITLEBAR_NO_MAXIMIZE : constant flags_t := 16#04#;

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return titlebar_access_t;
  pragma import (c, allocate, "AG_TitlebarNew");

  procedure set_caption
    (titlebar : titlebar_access_t;
     caption  : string);
  pragma inline (set_caption);

  function widget (titlebar : titlebar_access_t)
    return agar.gui.widget.widget_access_t
      renames agar.gui.types.widget_titlebar_widget;

end agar.gui.widget.titlebar;
