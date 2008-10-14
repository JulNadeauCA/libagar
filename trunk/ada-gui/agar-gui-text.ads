with agar.core.event;
with agar.core.object;
with agar.core.slist;
with agar.core.types;
with agar.gui.surface;
with interfaces.c.strings;

package agar.gui.text is
  package cs renames interfaces.c.strings;

  type glyph_t;
  type glyph_access_t is access all glyph_t;
  pragma convention (c, glyph_access_t);

  type font_t;
  type font_access_t is access all font_t;
  pragma convention (c, font_access_t);

  package glyph_slist is new agar.core.slist
    (entry_type => glyph_access_t);
  package font_slist is new agar.core.slist
    (entry_type => font_access_t);

  --
  -- types
  --

  type font_type_t is (VECTOR, BITMAP);
   for font_type_t use (VECTOR => 0, BITMAP => 1);
   for font_type_t'size use c.unsigned'size;
  pragma convention (c, font_type_t);

  type justify_t is (LEFT, CENTER, RIGHT);
   for justify_t use (LEFT => 0, CENTER => 1, RIGHT => 2);
   for justify_t'size use c.unsigned'size;
  pragma convention (c, justify_t);

  type valign_t is (TOP, MIDDLE, BOTTOM);
   for valign_t use (TOP => 0, MIDDLE => 1, BOTTOM => 2);
   for valign_t'size use c.unsigned'size;
  pragma convention (c, valign_t);

  type msg_title_t is (ERROR, WARNING, INFO);
   for msg_title_t use (ERROR => 0, WARNING => 1, INFO => 2);
   for msg_title_t'size use c.unsigned'size;
  pragma convention (c, msg_title_t);

  type font_name_t is array (1 .. agar.core.object.object_name_max) of aliased c.char;
  pragma convention (c, font_name_t);

  subtype flags_t is c.unsigned;
  FONT_BOLD      : constant flags_t := 16#01#;
  FONT_ITALIC    : constant flags_t := 16#02#;
  FONT_UNDERLINE : constant flags_t := 16#04#;
  FONT_UPPERCASE : constant flags_t := 16#08#;

  type spec_t is array (1 .. 32) of aliased c.char;
  pragma convention (c, spec_t);

  type font_t is record
    object    : agar.core.object.object_t;
    font_type : font_type_t;
    size      : c.int;
    flags     : flags_t;
    height    : c.int;
    ascent    : c.int;
    descent   : c.int;
    lineskip  : c.int;
    ttf       : agar.core.types.void_ptr_t;
    bspec     : spec_t;
    bglyphs   : access agar.gui.surface.surface_access_t;
    nglyphs   : c.unsigned;
    c0        : agar.core.types.uint32_t;
    c1        : agar.core.types.uint32_t;
    fonts     : font_slist.entry_t;
  end record;
  pragma convention (c, font_t);

  type state_t is record
    font      : font_access_t;
    color     : agar.core.types.uint32_t;
    color_bg  : agar.core.types.uint32_t;
    justify   : justify_t;
    valign    : valign_t;
  end record;
  type state_access_t is access all state_t;
  pragma convention (c, state_t);
  pragma convention (c, state_access_t);

  type static_font_t is record
    name      : cs.chars_ptr;
    font_type : font_type_t;
    size      : agar.core.types.uint32_t;
    data      : access agar.core.types.uint8_t;
    font      : font_access_t;
  end record;
  type static_font_access_t is access all font_t;
  pragma convention (c, static_font_t);
  pragma convention (c, static_font_access_t);

  type metrics_t is record
    width  : c.int;
    height : c.int;
    wlines : access c.unsigned;
    nlines : c.unsigned;
  end record;
  type metrics_access_t is access all metrics_t;
  pragma convention (c, metrics_t);
  pragma convention (c, metrics_access_t);

  -- openGL array types
  type texcoord_t is array (1 .. 4) of c.c_float;

  -- glyph type
  type glyph_t is record
    fontname : font_name_t;
    fontsize : c.int;
    color    : agar.core.types.uint32_t;
    ch       : agar.core.types.uint32_t;
    nrefs    : agar.core.types.uint32_t;
    last_ref : agar.core.types.uint32_t;
    surface  : agar.gui.surface.surface_access_t;
    advance  : c.int;

    -- openGL
    texture  : c.unsigned;
    texcoord : texcoord_t;

    glyphs   : glyph_slist.entry_t;
  end record;
  pragma convention (c, glyph_t);

  --
  -- API
  --

  procedure push_state;
  pragma import (c, push_state, "AG_PushTextState");

  procedure pop_state;
  pragma import (c, pop_state, "AG_PopTextState");

  procedure font (font : font_access_t);
  pragma import (c, font, "agar_gui_text_font");

  procedure justify (mode : justify_t);
  pragma import (c, justify, "agar_gui_text_justify");

  procedure color_video32 (pixel : agar.core.types.uint32_t);
  pragma import (c, color_video32, "AG_TextColorVideo32");

  procedure color_32 (pixel : agar.core.types.uint32_t);
  pragma import (c, color_32, "AG_TextColor32");

  procedure color_rgb
    (r : agar.core.types.uint8_t;
     g : agar.core.types.uint8_t;
     b : agar.core.types.uint8_t);
  pragma import (c, color_rgb, "AG_TextColorRGB");

  procedure color_rgba
    (r : agar.core.types.uint8_t;
     g : agar.core.types.uint8_t;
     b : agar.core.types.uint8_t;
     a : agar.core.types.uint8_t);
  pragma import (c, color_rgba, "AG_TextColorRGBA");
 
  procedure bg_color_video32 (pixel : agar.core.types.uint32_t);
  pragma import (c, bg_color_video32, "AG_TextBGColorVideo32");

  procedure bg_color_32 (pixel : agar.core.types.uint32_t);
  pragma import (c, bg_color_32, "AG_TextBGColor32");

  procedure bg_color_rgb
    (r : agar.core.types.uint8_t;
     g : agar.core.types.uint8_t;
     b : agar.core.types.uint8_t);
  pragma import (c, bg_color_rgb, "AG_TextBGColorRGB");

  procedure bg_color_rgba
    (r : agar.core.types.uint8_t;
     g : agar.core.types.uint8_t;
     b : agar.core.types.uint8_t;
     a : agar.core.types.uint8_t);
  pragma import (c, bg_color_rgba, "AG_TextBGColorRGBA");

  -- render

  function render (text : string) return agar.gui.surface.surface_access_t;
  pragma inline (render);
 
  function render_ucs4 (text : access c.char32_t)
    return agar.gui.surface.surface_access_t;
  pragma import (c, render_ucs4, "AG_TextRenderUCS4");

  function render_ucs4 (text : wide_wide_string)
    return agar.gui.surface.surface_access_t;
  pragma inline (render_ucs4);

  function render_glyph (glyph : agar.core.types.uint32_t)
    return agar.gui.surface.surface_access_t;
  pragma import (c, render_glyph, "AG_TextRenderGlyph");

  procedure size
    (text   : string;
     width  : out positive;
     height : out positive);
  pragma inline (size);

  procedure size_ucs4
    (text   : access c.char32_t;
     width  : access c.int;
     height : access c.int);
  pragma import (c, size_ucs4, "AG_TextSizeUCS4");

  procedure size_ucs4
    (text   : wide_wide_string;
     width  : out positive;
     height : out positive);
  pragma inline (size_ucs4);

  -- missing: AG_TextSizeMulti, AG_TextSizeMultiUCS4 - unsure how to bind this

  procedure msg
    (title : msg_title_t;
     text  : string);
  pragma inline (msg);

  procedure msg_from_error;
  pragma import (c, msg_from_error);

  procedure warning
    (disable_key : string;
     text        : string);
  pragma inline (warning);

  procedure info
    (disable_key : string;
     text        : string);
  pragma inline (info);

  procedure timed_message
    (title  : msg_title_t;
     expire : agar.core.types.uint32_t;
     text   : string);
  pragma inline (timed_message);
 
  -- missing: AG_TextEditFloat - unknown 'text_edit_float' enum type
  -- missing: AG_TextEditString - unsure how to bind

  procedure prompt_string
    (prompt  : string;
     ok_func : access procedure (event : agar.core.event.event_access_t);
     text    : string);
  pragma inline (prompt_string);

  procedure parse_font_spec (spec : string);
  pragma inline (parse_font_spec);

end agar.gui.text;
