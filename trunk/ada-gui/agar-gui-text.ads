with agar.core.event;
with agar.core.types;
with agar.gui.surface;

package agar.gui.text is

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

  --
  -- API
  --

  procedure push_state;
  pragma import (c, push_state, "AG_PushTextState");

  procedure pop_state;
  pragma import (c, pop_state, "AG_PopTextState");

  function font
    (face  : string;
     size  : positive;
     flags : c.unsigned) return boolean;
  pragma inline (font);

  procedure justify (mode : justify_t);
  pragma import (c, justify, "AG_TextJustify");

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

  procedure info (text : string);
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
