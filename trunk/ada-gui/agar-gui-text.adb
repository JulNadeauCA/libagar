package body agar.gui.text is

  use type c.int;

  package cbinds is
    function render (text : cs.chars_ptr) return agar.gui.surface.surface_access_t;
    pragma import (c, render, "agar_gui_text_render");
  
    function render_ucs4 (text : access c.char32_t) return agar.gui.surface.surface_access_t;
    pragma import (c, render_ucs4, "AG_TextRenderUCS4");

    procedure size
      (text   : cs.chars_ptr;
       width  : access c.int;
       height : access c.int);
    pragma import (c, size, "AG_TextSize");
 
    procedure size_ucs4
      (text   : access c.char32_t;
       width  : access c.int;
       height : access c.int);
    pragma import (c, size_ucs4, "AG_TextSizeUCS4");
  
    procedure msg
      (title : msg_title_t;
       text  : cs.chars_ptr);
    pragma import (c, msg, "AG_TextMsgS");
  
    procedure warning
      (disable_key : cs.chars_ptr;
       text        : cs.chars_ptr);
    pragma import (c, warning, "AG_TextWarningS");
    
    procedure info
      (disable_key : cs.chars_ptr;
       text        : cs.chars_ptr);
    pragma import (c, info, "AG_TextInfoS");
  
    procedure timed_message
      (title  : msg_title_t;
       expire : agar.core.types.uint32_t;
       text   : cs.chars_ptr);
    pragma import (c, timed_message, "AG_TextTmsgS");
  
    procedure prompt_string
      (prompt  : cs.chars_ptr;
       ok_func : access procedure (event : agar.core.event.event_access_t);
       fmt     : cs.chars_ptr;
       text    : cs.chars_ptr);
    pragma import (c, prompt_string, "AG_TextPromptString");
  
    procedure parse_font_spec (spec : cs.chars_ptr);
    pragma import (c, parse_font_spec, "AG_TextParseFontSpec");
  end cbinds;

  function render (text : string) return agar.gui.surface.surface_access_t is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    return cbinds.render (cs.to_chars_ptr (ca_text'unchecked_access));
  end render;

  -- this is probably as close to UCS4 as is possible here
  function render_ucs4 (text : wide_wide_string) return agar.gui.surface.surface_access_t is
    ca_text : aliased c.char32_array := c.to_c (text);
  begin
    return cbinds.render_ucs4 (ca_text (ca_text'first)'unchecked_access);
  end render_ucs4;

  procedure size
    (text   : string;
     width  : out positive;
     height : out positive)
  is
    c_width  : aliased c.int;
    c_height : aliased c.int;
    c_text   : aliased c.char_array := c.to_c (text);
  begin
    cbinds.size
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
    cbinds.size_ucs4
      (text   => c_text (c_text'first)'unchecked_access,
       height => c_height'unchecked_access,
       width  => c_width'unchecked_access);
    width := positive (c_width);
    height := positive (c_height);
  end size_ucs4;

  procedure msg
    (title : msg_title_t;
     text  : string)
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.msg
      (title => title,
       text  => cs.to_chars_ptr (ca_text'unchecked_access));
  end msg;

  procedure warning
    (disable_key : string;
     text        : string)
  is
    ca_disa : aliased c.char_array := c.to_c (disable_key);
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.warning
      (disable_key => cs.to_chars_ptr (ca_disa'unchecked_access),
       text        => cs.to_chars_ptr (ca_text'unchecked_access));
  end warning;

  procedure info
    (disable_key : string;
     text        : string)
  is
    ca_disa : aliased c.char_array := c.to_c (disable_key);
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.info
      (disable_key => cs.to_chars_ptr (ca_disa'unchecked_access),
       text        => cs.to_chars_ptr (ca_text'unchecked_access));
  end info;

  procedure timed_message
    (title  : msg_title_t;
     expire : agar.core.types.uint32_t;
     text   : string)
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.timed_message
      (title  => title,
       expire => expire,
       text   => cs.to_chars_ptr (ca_text'unchecked_access));
  end timed_message;
 
  procedure prompt_string
    (prompt  : string;
     ok_func : access procedure (event : agar.core.event.event_access_t);
     text    : string)
  is
    ca_fmt  : aliased c.char_array := c.to_c ("%s");
    ca_prmp : aliased c.char_array := c.to_c (prompt);
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    cbinds.prompt_string
      (prompt  => cs.to_chars_ptr (ca_prmp'unchecked_access),
       ok_func => ok_func,
       fmt     => cs.to_chars_ptr (ca_fmt'unchecked_access),
       text    => cs.to_chars_ptr (ca_text'unchecked_access));
  end prompt_string; 

  procedure parse_font_spec (spec : string) is
    ca_spec : aliased c.char_array := c.to_c (spec);
  begin
    cbinds.parse_font_spec (cs.to_chars_ptr (ca_spec'unchecked_access));
  end parse_font_spec;

end agar.gui.text;
