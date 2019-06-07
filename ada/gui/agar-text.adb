------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                            A G A R  . T E X T                            --
--                                  B o d y                                 --
--                                                                          --
-- Copyright (c) 2019 Julien Nadeau Carriere (vedge@csoft.net)              --
--                                                                          --
-- Permission to use, copy, modify, and/or distribute this software for any --
-- purpose with or without fee is hereby granted, provided that the above   --
-- copyright notice and this permission notice appear in all copies.        --
--                                                                          --
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES --
-- WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         --
-- MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  --
-- ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   --
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    --
-- ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  --
-- OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           --
------------------------------------------------------------------------------

package body Agar.Text is

  --
  -- Initialize the font engine.
  --
  function Init_Text_Subsystem return Boolean is
  begin
    return 0 = AG_InitTextSubsystem;
  end;
 
  --
  -- Release all resources allocated by the font engine.
  --
  procedure Destroy_Text_Subsystem is
  begin
    AG_DestroyTextSubsystem;
  end;

  --
  -- Set the default Agar font (by a font specification string).
  --
  -- Syntax: "(family):(size):(style)". Valid field separators include
  -- `:', `,', `.' and `/'. This works with fontconfig if available.
  -- Size is whole points (no fractional allowed with the default font).
  -- Style may include `b' (bold), `i' (italic) and `U' (uppercase).
  --
  procedure Set_Default_Font (Spec : in String)
  is
    Ch_Spec : aliased C.char_array := C.To_C(Spec);
  begin
    AG_TextParseFontSpec
      (Spec => CS.To_Chars_Ptr(Ch_Spec'Unchecked_Access));
  end;

  --
  -- Load (or fetch from cache) a font.
  --
  function Fetch_Font
    (Family     : in String         := "_agFontVera";
     Size       : in AG_Font_Points := AG_Font_Points(12);
     Bold       : in Boolean        := False;
     Italic     : in Boolean        := False;
     Underlined : in Boolean        := False;
     Uppercase  : in Boolean        := False) return Font_Access
  is
    Ch_Family : aliased C.char_array := C.To_C(Family);
    C_Size    : aliased AG_Font_Points := Size;
    Flags     : aliased C.unsigned := 0;
  begin
    if Bold       then Flags := Flags or FONT_BOLD;		end if;
    if Italic     then Flags := Flags or FONT_ITALIC;		end if;
    if Underlined then Flags := Flags or FONT_UNDERLINE;	end if;
    if Uppercase  then Flags := Flags or FONT_UPPERCASE;	end if;

    return AG_FetchFont
      (Family => CS.To_Chars_Ptr(Ch_Family'Unchecked_Access),
       Size   => C_Size'Unchecked_Access,
       Flags  => Flags);
  end;

  --
  -- Set the current font to the specified family+size+style (or just size).
  --
  function Set_Font
    (Family     : in String;
     Size       : in AG_Font_Points := AG_Font_Points(12);
     Bold       : in Boolean := False;
     Italic     : in Boolean := False;
     Underlined : in Boolean := False;
     Uppercase  : in Boolean := False) return Font_Access
  is
    Ch_Family : aliased C.char_array := C.To_C(Family);
    C_Size    : aliased AG_Font_Points := Size;
    Flags     : aliased C.unsigned := 0;
  begin
    if Bold       then Flags := Flags or FONT_BOLD;      end if;
    if Italic     then Flags := Flags or FONT_ITALIC;    end if;
    if Underlined then Flags := Flags or FONT_UNDERLINE; end if;
    if Uppercase  then Flags := Flags or FONT_UPPERCASE; end if;

    return AG_TextFontLookup
      (Family => CS.To_Chars_Ptr(Ch_Family'Unchecked_Access),
       Size   => C_Size'Unchecked_Access,
       Flags  => Flags);
  end;

  --
  -- Set the current font to a given % of the current font size.
  --
  function Set_Font (Percent : in Natural) return Font_Access is
  begin
    return AG_TextFontPct
      (Percent => C.int(Percent));
  end;  

  --
  -- Return the expected size in pixels of rendered (UTF-8) text.
  --
  procedure Size_Text
    (Text : in     String;
     W,H  :    out Natural)
  is
    Ch_Text  : aliased C.char_array := C.To_C(Text);
    C_W, C_H : aliased C.int;
  begin
    AG_TextSize
      (Text => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access),
       W    => C_W'Access,
       H    => C_H'Access);

    W := Natural(C_W);
    H := Natural(C_H);
  end;

  --
  -- Return the expected size in pixels of rendered (UTF-8) text,
  -- and the line count.
  --
  procedure Size_Text
    (Text       : in     String;
     W,H        :    out Natural;
     Line_Count :    out Natural)
  is
    Ch_Text  : aliased C.char_array := C.To_C(Text);
    C_W, C_H : aliased C.int;
    C_NLines : aliased C.unsigned;
  begin
    AG_TextSizeMulti
      (Text    => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access),
       W       => C_W'Access,
       H       => C_H'Access,
       W_Lines => null,
       N_Lines => C_NLines'Access);

    W := Natural(C_W);
    H := Natural(C_H);
    Line_Count := Natural(C_NLines);
  end;

  --
  -- Return the expected size in pixels of rendered (UTF-8) text,
  -- and the line count, and the width of each line.
  --
  procedure Size_Text
    (Text        : in     String;
     W,H         :    out Natural;
     Line_Count  :    out Natural;
     Line_Widths :    out Text_Line_Widths)
  is
    use Line_Width_Array;

    Ch_Text  : aliased C.char_array := C.To_C(Text);
    C_W, C_H : aliased C.int;
    C_WLines : aliased Line_Width_Array.Pointer;
    C_NLines : aliased C.unsigned;
  begin
    AG_TextSizeMulti
      (Text    => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access),
       W       => C_W'Access,
       H       => C_H'Access,
       W_Lines => C_WLines,
       N_Lines => C_NLines'Access);

    W := Natural(C_W);
    H := Natural(C_H);
    Line_Count := Natural(C_NLines);

    Line_Widths.Clear;

    for Index in 1 .. Line_Count loop
      declare
        Element : Natural;
      begin
        if C_WLines /= null then
          Element := Natural(C_WLines.all);
          Line_Widths.Append (Element);
        end if;
      end;
      Increment (C_WLines);
    end loop;
  end;

  --
  -- Display an informational message window (canned dialog).
  --
  procedure Message_Box
    (Title : in AG_Text_Message_Title := INFO;
     Text  : in String)
  is
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    AG_TextMsgS
      (Title => Title,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

#if AG_TIMERS
  procedure Message_Box
    (Title : in AG_Text_Message_Title := INFO;
     Text  : in String;
     Time  : in Natural := 2000)
  is
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    AG_TextTmsgS
      (Title => Title,
       Time  => Unsigned_32(Time),
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;
#end if;

end Agar.Text;
