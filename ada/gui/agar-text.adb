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
  -- Set the default font from a "<Face>:<Size>:<Flags>" string.
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
    (Family         : in String         := "_agFontVera";
     Size           : in AG_Font_Points := AG_Font_Points(12);
     Bold           : in Boolean        := False;
     Italic         : in Boolean        := False;
     Underlined     : in Boolean        := False;
     Uppercase      : in Boolean        := False;
     Semibold       : in Boolean        := False;
     Upright_Italic : in Boolean        := False;
     Semicondensed  : in Boolean        := False;
     Condensed      : in Boolean        := False) return Font_Access
  is
    Ch_Family : aliased C.char_array := C.To_C(Family);
    C_Size    : aliased AG_Font_Points := Size;
    Flags     : aliased C.unsigned := 0;
  begin
    if Bold           then Flags := Flags or FONT_BOLD;           end if;
    if Italic         then Flags := Flags or FONT_ITALIC;         end if;
    if Underlined     then Flags := Flags or FONT_UNDERLINE;      end if;
    if Uppercase      then Flags := Flags or FONT_UPPERCASE;      end if;
    if Semibold       then Flags := Flags or FONT_SEMIBOLD;       end if;
    if Upright_Italic then Flags := Flags or FONT_UPRIGHT_ITALIC; end if;
    if Semicondensed  then Flags := Flags or FONT_SEMICONDENSED;  end if;
    if Condensed      then Flags := Flags or FONT_CONDENSED;      end if;

    return AG_FetchFont
      (Family => CS.To_Chars_Ptr(Ch_Family'Unchecked_Access),
       Size   => C_Size'Unchecked_Access,
       Flags  => Flags);
  end;

  --
  -- Set the current font to the specified family+size+style (or just size).
  --
  procedure Text_Set_Font
    (Family         : in String         := "_agFontVera";
     Size           : in AG_Font_Points := AG_Font_Points(12);
     Bold           : in Boolean        := False;
     Italic         : in Boolean        := False;
     Underlined     : in Boolean        := False;
     Uppercase      : in Boolean        := False;
     Semibold       : in Boolean        := False;
     Upright_Italic : in Boolean        := False;
     Semicondensed  : in Boolean        := False;
     Condensed      : in Boolean        := False)
  is
    Ch_Family : aliased C.char_array := C.To_C(Family);
    C_Size    : aliased AG_Font_Points := Size;
    Flags     : aliased C.unsigned := 0;
    Result    : aliased Font_Access;
  begin
    if Bold           then Flags := Flags or FONT_BOLD;           end if;
    if Italic         then Flags := Flags or FONT_ITALIC;         end if;
    if Underlined     then Flags := Flags or FONT_UNDERLINE;      end if;
    if Uppercase      then Flags := Flags or FONT_UPPERCASE;      end if;
    if Semibold       then Flags := Flags or FONT_SEMIBOLD;       end if;
    if Upright_Italic then Flags := Flags or FONT_UPRIGHT_ITALIC; end if;
    if Semicondensed  then Flags := Flags or FONT_SEMICONDENSED;  end if;
    if Condensed      then Flags := Flags or FONT_CONDENSED;      end if;

    Result := AG_TextFontLookup
      (Family => CS.To_Chars_Ptr(Ch_Family'Unchecked_Access),
       Size   => C_Size'Unchecked_Access,
       Flags  => Flags);
  end;

  --
  -- Set the current font to a given % of the current font size.
  --
  procedure Text_Set_Font (Percent : in Natural)
  is
    Result : aliased Font_Access;
  begin
    Result := AG_TextFontPct
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
  -- Display an info, warning or error message.
  --
  procedure Text_Msg
    (Title : in AG_Text_Message_Title := INFO;
     Text  : in String)
  is
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    AG_TextMsgS
      (Title => Title,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

  --
  -- Display an info, warning or error message (for a # of milliseconds).
  --
#if AG_TIMERS
  procedure Text_Msg
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

  --
  -- Display an info message (with a "Don't tell me again" option
  -- tied to the Config setting named by Key).
  --
  procedure Text_Info (Key, Text : in String)
  is
    Ch_Key  : aliased C.char_array := C.To_C(Key);
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    AG_TextInfoS
      (Key  => CS.To_Chars_Ptr(Ch_Key'Unchecked_Access),
       Text => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

  --
  -- Display a warning message (with a "Don't tell me again" option
  -- tied to the Config setting named by Key).
  --
  procedure Text_Warning (Key, Text : in String)
  is
    Ch_Key  : aliased C.char_array := C.To_C(Key);
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    AG_TextWarningS
      (Key  => CS.To_Chars_Ptr(Ch_Key'Unchecked_Access),
       Text => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

  --
  -- Display an error message alert.
  --
  procedure Text_Error (Text : in String)
  is
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    AG_TextErrorS
      (Text => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

  --
  -- Calculate the X,Y offsets required to justify and vertically-align
  -- a text surface of a given size within a given area of pixels.
  --
  procedure Text_Align
    (W_Area, H_Area : in     Natural;
     W_Text, H_Text : in     Natural;
     L_Pad, R_Pad   : in     Natural := 0;
     T_Pad, B_Pad   : in     Natural := 0;
     Justify        : in     AG_Text_Justify := CENTER;
     Valign         : in     AG_Text_Valign := MIDDLE;
     X,Y            :    out Integer)
  is
    C_X, C_Y : aliased C.int;
  begin
    AG_TextAlign
      (X       => C_X'Access,
       Y       => C_Y'Access,
       W_Area  => C.int(W_Area),
       H_Area  => C.int(H_Area),
       W_Text  => C.int(W_Text),
       H_Text  => C.int(H_Text),
       L_Pad   => C.int(L_Pad),
       R_Pad   => C.int(R_Pad),
       T_Pad   => C.int(T_Pad),
       B_Pad   => C.int(B_Pad),
       Justify => Justify,
       Valign  => Valign);

    X := Integer(C_X);
    Y := Integer(C_Y);
  end;

  --
  -- Calculate the X offset required to justify rendered text in an area.
  --
  function Text_Justify (W_Area, W_Text : in Natural) return Integer
  is
    Result : C.int;
  begin
    Result := AG_TextJustifyOffset
      (W_Area => C.int(W_Area),
       W_Text => C.int(W_Text));
    return Integer(Result);
  end;

  --
  -- Calculate the Y offset required to vertically align rendered text
  -- in an area.
  --
  function Text_Valign (H_Area, H_Text : in Natural) return Integer
  is
    Result : C.int;
  begin
    Result := AG_TextValignOffset
      (H_Area => C.int(H_Area),
       H_Text => C.int(H_Text));
    return Integer(Result);
  end;

  --
  -- Set State Attribute: Tab Width (in pixels).
  --
  procedure Text_Set_Tab_Width (Width : Natural) is
  begin
    AG_TextTabWidth
      (Pixels => C.int(Width));
  end;

  --
  -- Render text to a new surface (UTF-8).
  --
  function Text_Render
    (Text : in String) return SU.Surface_not_null_Access
  is
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    return AG_TextRender
      (Text => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

  --
  -- Render text and blit it to an existing surface (UTF-8).
  --
  procedure Text_Render
    (Text    : in String;
     Surface : in SU.Surface_not_null_Access;
     X,Y     : in Natural := 0)
  is
    Ch_Text     : aliased C.char_array := C.To_C(Text);
    Tmp_Surface : constant SU.Surface_not_null_Access := AG_TextRender
      (Text => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  begin
    SU.Blit_Surface
      (Source => Tmp_Surface,
       Target => Surface,
       Dst_X  => X,
       Dst_Y  => Y);
    SU.Free_Surface
      (Surface => Tmp_Surface);
  end;

  --
  -- Render text and blit it to an existing surface (UCS-4 internal).
  --
  procedure Text_Render
    (Text    : in AG_Char_not_null_Access;
     Surface : in SU.Surface_not_null_Access;
     X,Y     : in Natural := 0)
  is
    Text_State  : aliased AG_Text_State;
    Tmp_Surface : SU.Surface_Access;
  begin
    Copy_Text_State(Text_State'Unchecked_Access);

    Tmp_Surface := Text_Render
      (Text     => Text,
       Font     => Text_State.Font,
       Color_BG => Text_State.Color_BG'Unchecked_Access,
       Color    => Text_State.Color'Unchecked_Access);

    SU.Blit_Surface
      (Source => Tmp_Surface,
       Target => Surface,
       Dst_X  => X,
       Dst_Y  => Y);

    SU.Free_Surface
      (Surface => Tmp_Surface);
  end;

end Agar.Text;
