package body agar.gui.widget.icon is

  package cbinds is
    function allocate_from_bitmap
      (filename : cs.chars_ptr) return icon_access_t;
    pragma import (c, allocate_from_bitmap, "AG_IconFromBMP");

    procedure set_text
      (icon : icon_access_t;
       str  : cs.chars_ptr);
    pragma import (c, set_text, "AG_IconSetTextS");

    procedure set_background_fill
      (icon  : icon_access_t;
       fill  : c.int;
       color : agar.core.types.uint32_t);
    pragma import (c, set_background_fill, "AG_IconSetBackgroundFill");

--    procedure set_padding
--      (icon   : icon_access_t;
--       left   : c.int;
--       right  : c.int;
--       top    : c.int;
--       bottom : c.int);
--    pragma import (c, set_padding, "AG_IconSetPadding");
  end cbinds;

  function allocate_from_bitmap
    (filename : string) return icon_access_t
  is
    ca_name : aliased c.char_array := c.to_c (filename);
  begin
    return cbinds.allocate_from_bitmap
      (filename => cs.to_chars_ptr (ca_name'unchecked_access));
  end allocate_from_bitmap;

  procedure set_text
    (icon : icon_access_t;
     text : string)
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.set_text
      (icon => icon,
       str  => cs.to_chars_ptr (ca_text'unchecked_access));
  end set_text;

--  procedure set_padding
--    (icon   : icon_access_t;
--     left   : natural;
--     right  : natural;
--     top    : natural;
--     bottom : natural) is
--  begin
--    cbinds.set_padding
--      (icon   => icon,
--       left   => c.int (left),
--       right  => c.int (right),
--       top    => c.int (top),
--       bottom => c.int (bottom));
--  end set_padding;

  procedure set_background_fill
    (icon  : icon_access_t;
     fill  : boolean;
     color : agar.core.types.uint32_t) is
    c_fill : c.int := 0;
  begin
    if fill then c_fill := 1; end if;
    cbinds.set_background_fill
      (icon  => icon,
       fill  => c_fill,
       color => color);
  end set_background_fill;

end agar.gui.widget.icon;
