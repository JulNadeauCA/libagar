package body agar.gui.widget.combo is

  package cbinds is
    function allocate
      (parent : widget_access_t;
       flags  : flags_t;
       label  : cs.chars_ptr) return combo_access_t;
    pragma import (c, allocate, "AG_ComboNewS");
  
    procedure size_hint
      (combo  : combo_access_t;
       text   : cs.chars_ptr;
       items  : c.int);
    pragma import (c, size_hint, "AG_ComboSizeHint");
  
    procedure size_hint_pixels
      (combo  : combo_access_t;
       width  : c.int;
       height : c.int);
    pragma import (c, size_hint_pixels, "AG_ComboSizeHintPixels");
  
    procedure select_text
      (combo  : combo_access_t;
       text   : cs.chars_ptr);
    pragma import (c, select_text, "AG_ComboSelectText");
  end cbinds;

  --

  function allocate
    (parent : widget_access_t;
     flags  : flags_t;
     label  : string) return combo_access_t
  is
    ca_text : aliased c.char_array := c.to_c (label);
  begin
    return cbinds.allocate
      (parent => parent,
       flags  => flags,
       label  => cs.to_chars_ptr (ca_text'unchecked_access));
  end allocate;

  procedure size_hint
    (combo  : combo_access_t;
     text   : string;
     items  : integer)
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.size_hint
      (combo => combo,
       text  => cs.to_chars_ptr (ca_text'unchecked_access),
       items => c.int (items));
  end size_hint;

  procedure size_hint_pixels
    (combo  : combo_access_t;
     width  : positive;
     height : positive) is
  begin
    cbinds.size_hint_pixels
      (combo  => combo,
       width  => c.int (width),
       height => c.int (height));
  end size_hint_pixels;

  procedure select_text
    (combo : combo_access_t;
     text  : string) is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.select_text
      (combo => combo,
       text  => cs.to_chars_ptr (ca_text'unchecked_access));
  end select_text;

end agar.gui.widget.combo;
