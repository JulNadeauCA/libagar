package body agar.gui.widget.ucombo is

  package cbinds is
    function allocate_polled
      (parent   : widget_access_t;
       flags    : flags_t;
       callback : agar.core.event.callback_t;
       fmt      : agar.core.types.void_ptr_t) return ucombo_access_t; 
    pragma import (c, allocate_polled, "AG_UComboNewPolled");

    procedure size_hint
      (ucombo : ucombo_access_t;
       text   : cs.chars_ptr;
       items  : c.int);
    pragma import (c, size_hint, "AG_UComboSizeHint");

    procedure size_hint_pixels
      (ucombo : ucombo_access_t;
       width  : c.int;
       height : c.int);
    pragma import (c, size_hint_pixels, "AG_UComboSizeHintPixels");
  end cbinds;

  function allocate_polled
    (parent   : widget_access_t;
     flags    : flags_t;
     callback : agar.core.event.callback_t) return ucombo_access_t is
  begin
    return cbinds.allocate_polled
      (parent   => parent,
       flags    => flags,
       callback => callback,
       fmt      => agar.core.types.null_ptr);
  end allocate_polled;

  procedure size_hint
    (ucombo : ucombo_access_t;
     text   : string;
     items  : natural)
  is
    c_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.size_hint
      (ucombo => ucombo,
       text   => cs.to_chars_ptr (c_text'unchecked_access),
       items  => c.int (items));
  end size_hint;

  procedure size_hint_pixels
    (ucombo : ucombo_access_t;
     width  : natural;
     height : natural) is
  begin
    cbinds.size_hint_pixels
      (ucombo => ucombo,
       width  => c.int (width),
       height => c.int (height));
  end size_hint_pixels;

  --            

  function widget (ucombo : ucombo_access_t) return widget_access_t is
  begin
    return ucombo.widget'access;
  end widget;

end agar.gui.widget.ucombo;
