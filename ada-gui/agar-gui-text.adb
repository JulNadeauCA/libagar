package body agar.gui.text is

  use type c.int;

  function font
    (face  : cs.chars_ptr;
     size  : c.int;
     flags : c.unsigned) return c.int;
  pragma import (c, font, "AG_TextFont");

  function font
    (face  : string;
     size  : positive;
     flags : c.unsigned) return boolean
  is
    ca_face : aliased c.char_array := c.to_c (face);
  begin
    return font
      (face  => cs.to_chars_ptr (ca_face'unchecked_access),
       size  => c.int (size),
       flags => flags) = 0;
  end font;

  function render (text : cs.chars_ptr) return agar.gui.surface.surface_access_t;
  pragma import (c, render, "AG_TextRender");

  function render (text : string) return agar.gui.surface.surface_access_t is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    return render (cs.to_chars_ptr (ca_text'unchecked_access));
  end render;

  -- this is probably as close to UCS4 as is possible here
  function render_ucs4 (text : wide_wide_string) return agar.gui.surface.surface_access_t is
    ca_text : aliased c.char32_array := c.to_c (text);
  begin
    return render_ucs4 (ca_text (ca_text'first)'unchecked_access);
  end render_ucs4;

  procedure size
    (text   : cs.chars_ptr;
     width  : access c.int;
     height : access c.int);
  pragma import (c, size, "AG_TextSize");

  procedure size
    (text   : string;
     width  : out positive;
     height : out positive)
  is
    c_width  : aliased c.int;
    c_height : aliased c.int;
    c_text   : aliased c.char_array := c.to_c (text);
  begin
    size
      (text   => cs.to_chars_ptr (c_text'unchecked_access),
       height => c_height'unchecked_access,
       width  => c_width'unchecked_access);
    width := positive (c_width);
    height := positive (c_height);
  end size;

  procedure size_ucs4
    (text   : wide_wide_string;
     width  : out positive;
     height : out positive)
  is
    c_width  : aliased c.int;
    c_height : aliased c.int;
    c_text   : aliased c.char32_array := c.to_c (text);
  begin
    size_ucs4
      (text   => c_text (c_text'first)'unchecked_access,
       height => c_height'unchecked_access,
       width  => c_width'unchecked_access);
    width := positive (c_width);
    height := positive (c_height);
  end size_ucs4;

  procedure msg
    (title : msg_title_t;
     fmt   : cs.chars_ptr;
     text  : cs.chars_ptr);
  pragma import (c, msg, "AG_TextMsg");

  procedure msg
    (title : msg_title_t;
     text  : string)
  is
    ca_fmt  : aliased c.char_array := c.to_c ("%s");
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    msg
      (title => title,
       fmt   => cs.to_chars_ptr (ca_fmt'unchecked_access),
       text  => cs.to_chars_ptr (ca_text'unchecked_access));
  end msg;

  procedure warning
    (disable_key : cs.chars_ptr;
     fmt         : cs.chars_ptr;
     text        : cs.chars_ptr);
  pragma import (c, warning, "AG_TextWarning");

  procedure warning
    (disable_key : string;
     text        : string)
  is
    ca_fmt  : aliased c.char_array := c.to_c ("%s");
    ca_disa : aliased c.char_array := c.to_c (disable_key);
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    warning
      (disable_key => cs.to_chars_ptr (ca_disa'unchecked_access),
       fmt         => cs.to_chars_ptr (ca_fmt'unchecked_access),
       text        => cs.to_chars_ptr (ca_text'unchecked_access));
  end warning;

  procedure info
    (fmt  : cs.chars_ptr;
     text : cs.chars_ptr);
  pragma import (c, info, "AG_TextInfo");

  procedure info (text : string) is
    ca_fmt  : aliased c.char_array := c.to_c ("%s");
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    info
      (fmt  => cs.to_chars_ptr (ca_fmt'unchecked_access),
       text => cs.to_chars_ptr (ca_text'unchecked_access));
  end info;

  procedure timed_message
    (title  : msg_title_t;
     expire : agar.core.types.uint32_t;
     fmt    : cs.chars_ptr;
     text   : cs.chars_ptr);
  pragma import (c, timed_message, "AG_TextTmsg");

  procedure timed_message
    (title  : msg_title_t;
     expire : agar.core.types.uint32_t;
     text   : string)
  is
    ca_fmt  : aliased c.char_array := c.to_c ("%s");
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    timed_message
      (title  => title,
       expire => expire,
       fmt    => cs.to_chars_ptr (ca_fmt'unchecked_access),
       text   => cs.to_chars_ptr (ca_text'unchecked_access));
  end timed_message;
 
  procedure prompt_string
    (prompt  : cs.chars_ptr;
     ok_func : access procedure (event : agar.core.event.event_access_t);
     fmt     : cs.chars_ptr;
     text    : cs.chars_ptr);
  pragma import (c, prompt_string, "AG_TextPromptString");

  procedure prompt_string
    (prompt  : string;
     ok_func : access procedure (event : agar.core.event.event_access_t);
     text    : string)
  is
    ca_fmt  : aliased c.char_array := c.to_c ("%s");
    ca_prmp : aliased c.char_array := c.to_c (prompt);
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    prompt_string
      (prompt  => cs.to_chars_ptr (ca_prmp'unchecked_access),
       ok_func => ok_func,
       fmt     => cs.to_chars_ptr (ca_fmt'unchecked_access),
       text    => cs.to_chars_ptr (ca_text'unchecked_access));
  end prompt_string; 

  procedure parse_font_spec (spec : cs.chars_ptr);
  pragma import (c, parse_font_spec, "AG_TextParseFontSpec");

  procedure parse_font_spec (spec : string) is
    ca_spec : aliased c.char_array := c.to_c (spec);
  begin
    parse_font_spec (cs.to_chars_ptr (ca_spec'unchecked_access));
  end parse_font_spec;

end agar.gui.text;
