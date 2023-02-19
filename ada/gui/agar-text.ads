------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                            A G A R .  T E X T                            --
--                                  S p e c                                 --
------------------------------------------------------------------------------
with Ada.Containers.Indefinite_Vectors;
with Interfaces; use Interfaces;
with Interfaces.C;
with Interfaces.C.Pointers;
with Interfaces.C.Strings;
with Agar.Types; use Agar.Types;
with Agar.Object;
with Agar.Surface;
with Agar.Widget;
with System;

package Agar.Text is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;
  package SU renames Agar.Surface;
  package WID renames Agar.Widget;

  use type C.int;
  use type C.unsigned;

  -- Limits --
  TEXT_STATES_MAX     : constant C.unsigned := $AG_TEXT_STATES_MAX;

  -- Agar Font style flags --
  FONT_THIN           : constant C.unsigned := 16#00_01#;
  FONT_EXTRALIGHT     : constant C.unsigned := 16#00_02#;
  FONT_LIGHT          : constant C.unsigned := 16#00_04#;
  FONT_SEMIBOLD       : constant C.unsigned := 16#00_08#;
  FONT_BOLD           : constant C.unsigned := 16#00_10#;
  FONT_EXTRABOLD      : constant C.unsigned := 16#00_20#;
  FONT_BLACK          : constant C.unsigned := 16#00_40#;
  FONT_OBLIQUE        : constant C.unsigned := 16#00_80#;
  FONT_ITALIC         : constant C.unsigned := 16#01_00#;
  FONT_ULTRACONDENSED : constant C.unsigned := 16#04_00#;
  FONT_CONDENSED      : constant C.unsigned := 16#08_00#;
  FONT_SEMICONDENSED  : constant C.unsigned := 16#10_00#;
  FONT_SEMIEXPANDED   : constant C.unsigned := 16#20_00#;
  FONT_EXPANDED       : constant C.unsigned := 16#40_00#;
  FONT_ULTRAEXPANDED  : constant C.unsigned := 16#80_00#;

  FONT_WEIGHTS        : constant C.unsigned := 16#00_7f#;  -- All weights 
  FONT_STYLES         : constant C.unsigned := 16#01_80#;  -- All styles
  FONT_WIDTH_VARIANTS : constant C.unsigned := 16#fc_00#;  -- All wd. variants

  -- Agar Font state flags --
  FONT_FONTCONFIGED : constant C.unsigned := 16#01#;  -- Discovered via fontconfig
  FONT_FAMILY_FLAGS : constant C.unsigned := 16#02#;  -- Family flags are specified

  -----------------------------
  -- Text Justification Mode --
  -----------------------------
  type Text_Justify is
    (LEFT,
     CENTER,
     RIGHT);
  for Text_Justify use
    (LEFT   => 0,
     CENTER => 1,
     RIGHT  => 2);
  for Text_Justify'Size use C.int'Size;
  
  ----------------------------------
  -- Vertical Text Alignment Mode --
  ----------------------------------
  type Text_Valign is
    (TOP,
     MIDDLE,
     BOTTOM);
  for Text_Valign use
    (TOP    => 0,
     MIDDLE => 1,
     BOTTOM => 2);
  for Text_Valign'Size use C.int'Size;

  --------------------------------
  -- Type of message to display --
  --------------------------------
  type Text_Message_Title is
    (ERROR,                       -- Error message alert
     WARNING,                     -- Warning (ignorable)
     INFO);                       -- Informational message (ignorable)
  for Text_Message_Title use
    (ERROR   => 0,
     WARNING => 1,
     INFO    => 2);
  for Text_Message_Title'Size use C.int'Size;

  ------------------
  -- Type of font --
  ------------------
  type Font_Engine_Type is
    (FREETYPE,           -- FreeType vector font engine
     BITMAP,             -- Agar bitmap font engine 
     DUMMY);             -- A "no-op" font engine
  for Font_Engine_Type use
    (FREETYPE => 0,
     BITMAP   => 1,
     DUMMY    => 2);
  for Font_Engine_Type'Size use C.int'Size;
  
  -------------------------------------------
  -- Type of data source to load font from --
  -------------------------------------------
  type Font_Spec_Source is
    (FONT_FILE,                       -- Load font from file
     FONT_IN_MEMORY);                 -- Deserialize in-memory font data
  for Font_Spec_Source use
    (FONT_FILE      => 0,
     FONT_IN_MEMORY => 1);
  for Font_Spec_Source'Size use C.int'Size;
 
  ----------------------------
  -- Size of font in points --
  ----------------------------
  subtype Font_Points is C.C_float;
  type Font_Points_Access is access all Font_Points with Convention => C;

  -----------------------------
  -- Agar font specification --
  -----------------------------
  type Font_Spec
    (Spec_Source : Font_Spec_Source := FONT_FILE) is
  record
    Size        : Font_Points;              -- Font size in points
    Index       : C.int;                    -- Font index (FC_INDEX)
    Font_Type   : Font_Engine_Type;         -- Font engine type
    Font_Source : Font_Spec_Source;         -- Source type

    Matrix_XX   : C.double;       -- 1 --   -- Transformation matrix
    Matrix_XY   : C.double;       -- 0 --
    Matrix_YX   : C.double;       -- 0 --
    Matrix_YY   : C.double;       -- 1 --

    case Spec_Source is
    when FONT_FILE =>
      null;
    when FONT_IN_MEMORY =>
      Memory_Source : access Unsigned_8;    -- Source memory region
      Memory_Size   : AG_Size;              -- Size in bytes
#if AG_MODEL = AG_MEDIUM
      C_Pad1        : Interfaces.Unsigned_32;
#end if;
    end case;
  end record
    with Convention => C;
  pragma Unchecked_Union (Font_Spec);

  type Font_Spec_Access is access all Font_Spec with Convention => C;
  subtype Font_Spec_not_null_Access is not null Font_Spec_Access;

  ------------------
  -- An Agar Font --
  ------------------
  type Font;
  type Font_Access is access all Font with Convention => C;
  subtype Font_not_null_Access is not null Font_Access;

  type Font_Entry is limited record
    Next : Font_Access;
    Prev : access Font_Access;
  end record
    with Convention => C;

  type Font_Name is array (1 .. Agar.Object.NAME_MAX) of aliased c.char
      with Convention => C;
  type Font_Family_Style_Access is access all C.unsigned with Convention => C;
  subtype Font_Family_Style_not_null_Access is not null Font_Family_Style_Access;

  type Font is limited record
    Super              : aliased Agar.Object.Object;
    Name               : aliased Font_Name;         -- Font family
    Spec               : aliased Font_Spec;         -- Font specification
    Flags              : C.unsigned;                -- Style/Weight/Wd.Variant
    Family_Style_Count : C.unsigned;                -- No. of styles in font's family
    Family_Styles      : Font_Family_Style_Access;  -- Styles in font's family
    State_Flags        : C.unsigned;                -- State flags
    Height             : C.int;                     -- Height (px)
    Ascent             : C.int;                     -- Ascent (px)
    Descent            : C.int;                     -- Descent (px)
    Line_Skip          : C.int;                     -- Multiline y-increment (px)
    Underline_Pos      : C.int;                     -- Underline position
    Underline_Thick    : C.int;                     -- Underline thickness
    Access_Time        : C.unsigned;                -- Access time (debug mode only)
    Entry_in_Fonts     : Font_Entry;                -- Entry in global fonts list
  end record
    with Convention => C;

  ----------------------------------
  -- A rendered (in-memory) glyph --
  ----------------------------------
  type Glyph;
  type Glyph_Access is access all Glyph with Convention => C;
  subtype Glyph_not_null_Access is not null Glyph_Access;
  type Glyph_Entry is limited record
    Next : Glyph_Access;
  end record
    with Convention => C;
  type Glyph is limited record
    Font           : Font_not_null_Access;       -- Font face
    Color_BG       : SU.AG_Color;                -- Background color
    Color          : SU.AG_Color;                -- Foreground color
    Surface        : SU.Surface_not_null_Access; -- Rendered surface
    Char           : AG_Char;                    -- Native character
    Advance        : C.int;                      -- Advance in pixels
    Texture        : C.unsigned;                 -- Mapped texture (by driver)
    Texcoords      : SU.AG_Texcoord;             -- Texture coordinates
    C_Pad1         : Interfaces.Unsigned_32;
    Entry_in_Cache : Glyph_Entry;                -- Entry in cache
  end record
    with Convention => C;

  ---------------------------------------
  -- Pushable/poppable state variables --
  ---------------------------------------
#if AG_DEBUG
  type Text_State_Tag is array (1 .. 8) of aliased C.char
    with Convention => C;
#end if;
  type Text_State_Rec is record
    Font       : Font_not_null_Access;    -- Font face
    Color      : aliased SU.AG_Color;     -- Foreground text color
    Color_BG   : aliased SU.AG_Color;     -- Background color
    Color_ANSI : SU.Color_Access;         -- ANSI color palette (3/4-bit)
    Justify    : Text_Justify;            -- Justification mode
    Valign     : Text_Valign;             -- Vertical alignment
    Tab_Width  : C.int;                   -- Width of tabs in pixels
    C_Pad1     : Unsigned_32;
  end record
    with Convention => C;
  type Text_State_Access is access all Text_State_Rec with Convention => C;
  subtype Text_State_not_null_Access is not null Text_State_Access;
 
  ------------------------------------------
  -- Statically-compiled font description --
  ------------------------------------------
  type Static_Font is array (1 .. $SIZEOF_AG_StaticFont)
    of aliased Unsigned_8 with Convention => C;
  for Static_Font'Size use $SIZEOF_AG_StaticFont * System.Storage_Unit;
  
  ------------------------------
  -- Measure of rendered text --
  ------------------------------
  type Text_Metrics is record
    W,H         : C.int;                -- Dimensions in pixels
    Line_Widths : access C.unsigned;    -- Width of each line
    Line_Count  : C.unsigned;           -- Total line count
    C_Pad1      : Unsigned_32;
  end record
    with Convention => C;
  type Text_Metrics_Access is access all Text_Metrics with Convention => C;
  subtype Text_Metrics_not_null_Access is not null Text_Metrics_Access;

  package Text_Line_Widths_Packages is new Ada.Containers.Indefinite_Vectors
    (Index_Type   => Positive,
     Element_Type => Natural);
  subtype Text_Line_Widths is Text_Line_Widths_Packages.Vector;

  --------------------------
  -- Internal glyph cache --
  --------------------------
  type Glyph_Cache is array (1 .. $SIZEOF_AG_GlyphCache)
    of aliased Unsigned_8 with Convention => C;
  for Glyph_Cache'Size use $SIZEOF_AG_GlyphCache * System.Storage_Unit;

  --
  -- Initialize the font engine.
  --
  function Init_Text_Subsystem return Boolean;

  --
  -- Release all resources allocated by the font engine.
  --
  procedure Destroy_Text_Subsystem;
  
  --
  -- Set the default Agar font (by access to a Font object).
  -- Return a pointer to the previous default font.
  -- Updates the default font settings in AG_Config(3).
  --
  function Set_Default_Font (Font : in Font_not_null_Access) return Font_Access
    with Import, Convention => C, Link_Name => "AG_SetDefaultFont";

  --
  -- Set the default font from a string of the form "<Face>,<Size>,<Style>".
  -- Updates the default font settings in AG_Config(3).
  --
  procedure Set_Default_Font (Spec : in String);

  --
  -- Load the given font (or return a pointer to an existing one), from
  -- a specified font face, size and style.
  --
  -- Font family names are case-insensitive and may correspond to either
  -- fontconfig-managed font names or font files installed under the
  -- PATH_FONTS of AG_Config(3).
  --
  function Fetch_Font
    (Family         : in String      := "algue";
     Size           : in Font_Points := Font_Points(12);
     Thin           : in Boolean     := False;    -- Wt# 100
     ExtraLight     : in Boolean     := False;    -- Wt# 200
     Light          : in Boolean     := False;    -- Wt# 300          
     SemiBold       : in Boolean     := False;    -- Wt# 600
     Bold           : in Boolean     := False;    -- Wt# 700
     ExtraBold      : in Boolean     := False;    -- Wt# 800
     Black          : in Boolean     := False;    -- Wt# 900
     Oblique        : in Boolean     := False;    -- With italic fallback
     Italic         : in Boolean     := False;    -- With oblique fallback
     UltraCondensed : in Boolean     := False;    -- Wd. 50%
     Condensed      : in Boolean     := False;    -- Wd. 75%
     SemiCondensed  : in Boolean     := False;    -- Wd. 87.5%
     SemiExpanded   : in Boolean     := False;    -- Wd. 112.5%
     Expanded       : in Boolean     := False;    -- Wd. 125%
     UltraExpanded  : in Boolean     := False)    -- Wd. 200%
      return Font_Access;

  --
  -- Decrement the reference count of a font (and free unreferenced fonts).
  --
  procedure Unused_Font (Font : in Font_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_UnusedFont";

  --
  -- Push and pop the font engine rendering state.
  --
  procedure Push_Text_State
    with Import, Convention => C, Link_Name => "AG_PushTextState";
  procedure Pop_Text_State
    with Import, Convention => C, Link_Name => "AG_PopTextState";
  procedure Copy_Text_State
    (Target : in Text_State_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_CopyTextState";

  --
  -- Set the current font to the specified family+size+style (or just size).
  --
  procedure Text_Set_Font
    (Family         : in String      := "algue";
     Size           : in Font_Points := Font_Points(12);
     Thin           : in Boolean     := False;
     ExtraLight     : in Boolean     := False;
     Light          : in Boolean     := False;
     SemiBold       : in Boolean     := False;
     Bold           : in Boolean     := False;
     ExtraBold      : in Boolean     := False;
     Black          : in Boolean     := False;
     Oblique        : in Boolean     := False;
     Italic         : in Boolean     := False;
     UltraCondensed : in Boolean     := False;
     Condensed      : in Boolean     := False;
     SemiCondensed  : in Boolean     := False;
     SemiExpanded   : in Boolean     := False;
     Expanded       : in Boolean     := False;
     UltraExpanded  : in Boolean     := False);

  --
  -- Set the current font to a given % of the current font size.
  --
  procedure Text_Set_Font (Percent : in Natural);
  
  --
  -- Set the current font to the referenced font.
  --
  procedure Text_Set_Font (Font : Font_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TextFont";
  
  --
  -- Return the expected size in pixels of rendered (UTF-8) text.
  --
  procedure Size_Text
    (Text : in     String;
     W,H  :    out Natural);
  procedure Size_Text
    (Text       : in     String;
     W,H        :    out Natural;
     Line_Count :    out Natural);
  procedure Size_Text
    (Text        : in     String;
     W,H         :    out Natural;
     Line_Count  :    out Natural;
     Line_Widths :    out Text_Line_Widths);

  --
  -- Canned dialogs: Display an informational, warning or error message.
  --
  procedure Text_Msg
    (Title : in Text_Message_Title := INFO;
     Text  : in String);
#if AG_TIMERS
  procedure Text_Msg
    (Title : in Text_Message_Title := INFO;
     Text  : in String;
     Time  : in Natural := 2000);
#end if;
  procedure Text_Msg_From_Error
    with Import, Convention => C, Link_Name => "AG_TextMsgFromError";
  procedure Text_Info (Key, Text : in String);
  procedure Text_Warning (Key, Text : in String);
  procedure Text_Error (Text : in String);

  --
  -- Calculate the X,Y offsets required to justify and vertically-align
  -- a text surface of a given size within a given area of pixels.
  --
  procedure Text_Align
    (W_Area, H_Area : in     Natural;
     W_Text, H_Text : in     Natural;
     L_Pad, R_Pad   : in     Natural := 0;
     T_Pad, B_Pad   : in     Natural := 0;
     Justify        : in     Text_Justify := CENTER;
     Valign         : in     Text_Valign := MIDDLE;
     X,Y            :    out Integer);

  --
  -- Render text to a new surface.
  --
  function Text_Render
    (Text : in String) return SU.Surface_not_null_Access;
  function Text_Render_RTL
    (Text : in String) return SU.Surface_not_null_Access;
  function Text_Render
    (Text     : in AG_Char_not_null_Access;
     Font     : in Font_not_null_Access;
     Color_BG : in SU.Color_not_null_Access;
     Color    : in SU.Color_not_null_Access) return SU.Surface_not_null_Access
    with Import, Convention => C, Link_Name => "AG_TextRenderInternal";

  --
  -- Render text and blit it to an existing surface.
  --
  procedure Text_Render
    (Text    : in String;                     -- UTF-8
     Surface : in SU.Surface_not_null_Access;
     X,Y     : in Natural := 0);
  procedure Text_Render
    (Text    : in AG_Char_not_null_Access;    -- UCS-4 (internal)
     Surface : in SU.Surface_not_null_Access;
     X,Y     : in Natural := 0);

  --
  -- Lookup (possibly bringing into cache), a glyph.
  --
  function Text_Render_Glyph
    (Driver   : in WID.Driver_not_null_Access;
     Font     : in Font_not_null_Access;
     Color_BG : in SU.Color_not_null_Access;
     Color    : in SU.Color_not_null_Access;
     Char     : in AG_Char) return Glyph_not_null_Access
    with Import, Convention => C, Link_Name => "AG_TextRenderGlyph";

  --
  -- Set State Attribute: Foreground Color.
  --
  procedure Text_Set_Color
    (Color : in SU.Color_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TextColor";
  procedure Text_Set_Color_8
    (R,G,B : in Unsigned_8;
     A     : in Unsigned_8 := 255)
    with Import, Convention => C, Link_Name => "AG_TextColorRGBA";
  procedure Text_Set_Color_8
    (RGBA : in Unsigned_32)
    with Import, Convention => C, Link_Name => "AG_TextColorHex";

  --
  -- Set State Attribute: Background Color.
  --
  procedure Text_Set_BG_Color
    (Color : in SU.Color_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TextBGColor";
  procedure Text_Set_BG_Color_8
    (R,G,B : in Unsigned_8;
     A     : in Unsigned_8 := 255)
    with Import, Convention => C, Link_Name => "AG_TextBGColorRGBA";
  procedure Text_Set_BG_Color_8
    (RGBA : in Unsigned_32)
    with Import, Convention => C, Link_Name => "AG_TextBGColorHex";

  --
  -- Set State Attributes: Justification, Alignment, Tab width.
  --
  procedure Text_Set_Justify
    (Justify : Text_Justify)
    with Import, Convention => C, Link_Name => "AG_TextJustify";
  procedure Text_Set_Valign
    (Valign : Text_Valign)
    with Import, Convention => C, Link_Name => "AG_TextValign";
  procedure Text_Set_Tab_Width
    (Width : Natural);

  private

  function AG_InitTextSubsystem return C.int
    with Import, Convention => C, Link_Name => "AG_InitTextSubsystem";

  procedure AG_DestroyTextSubsystem
    with Import, Convention => C, Link_Name => "AG_DestroyTextSubsystem";

  procedure AG_TextParseFontSpec
    (Spec : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_TextParseFontSpec";

  function AG_FetchFont
    (Family : in CS.chars_ptr;
     Size   : in Font_Points_Access;
     Flags  : in C.unsigned) return Font_Access
    with Import, Convention => C, Link_Name => "AG_FetchFont";

  function AG_TextFontLookup
    (Family : in CS.chars_ptr;
     Size   : in Font_Points_Access;
     Flags  : in C.unsigned) return Font_Access
    with Import, Convention => C, Link_Name => "AG_TextFontLookup";

  function AG_TextFontPct
    (Percent : in C.int) return Font_Access
    with Import, Convention => C, Link_Name => "AG_TextFontPct";

  procedure AG_TextSize
    (Text : in CS.chars_ptr;
     W,H  : access C.int)
    with Import, Convention => C, Link_Name => "AG_TextSize";

  type AG_TextSizeMulti_Line_Entry is array (C.unsigned range <>)
      of aliased C.unsigned with Convention => C;

  package Line_Width_Array is new Interfaces.C.Pointers
    (Index              => C.unsigned,
     Element            => C.unsigned,
     Element_Array      => AG_TextSizeMulti_Line_Entry,
     Default_Terminator => 0);

  procedure AG_TextSizeMulti
    (Text    : in CS.chars_ptr;
     W,H     : access C.int;
     W_Lines : in Line_Width_Array.Pointer;
     N_Lines : access C.unsigned)
    with Import, Convention => C, Link_Name => "AG_TextSizeMulti";

  procedure AG_TextMsgS
    (Title : in Text_Message_Title;
     Text  : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_TextMsgS";
#if AG_TIMERS
  procedure AG_TextTmsgS
    (Title : in Text_Message_Title;
     Time  : in Unsigned_32;
     Text  : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_TextTmsgS";
#end if;

  procedure AG_TextInfoS (Key, Text : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_TextInfoS";

  procedure AG_TextWarningS (Key, Text : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_TextWarningS";

  procedure AG_TextErrorS (Text : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_TextErrorS";

  procedure AG_TextAlign
    (X,Y            : access C.int;
     W_Area, H_Area : in C.int;
     W_Text, H_Text : in C.int;
     L_Pad, R_Pad   : in C.int;
     T_Pad, B_Pad   : in C.int;
     Justify        : in Text_Justify;
     Valign         : in Text_Valign)
    with Import, Convention => C, Link_Name => "AG_TextAlign";

  procedure AG_TextTabWidth (Pixels : in C.int)
    with Import, Convention => C, Link_Name => "AG_TextTabWidth";

  function AG_TextRender (Text : in CS.chars_ptr) return SU.Surface_not_null_Access
    with Import, Convention => C, Link_Name => "AG_TextRender";

  function AG_TextRenderRTL (Text : in CS.chars_ptr) return SU.Surface_not_null_Access
    with Import, Convention => C, Link_Name => "AG_TextRenderRTL";

end Agar.Text;
