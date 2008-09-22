package body agar.gui.widget.titlebar is

  package cbinds is 
    procedure set_caption
      (titlebar : titlebar_access_t;
       caption  : cs.chars_ptr);
    pragma import (c, set_caption, "AG_TitlebarSetCaption");
  end cbinds;

  procedure set_caption
    (titlebar : titlebar_access_t;
     caption  : string)
  is
    c_caption : aliased c.char_array := c.to_c (caption);
  begin
    cbinds.set_caption
      (titlebar => titlebar,
       caption  => cs.to_chars_ptr (c_caption'unchecked_access));
  end set_caption;

end agar.gui.widget.titlebar;
