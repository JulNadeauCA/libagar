------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                           A G A R  . C O M B O                           --
--                                 B o d y                                  --
--                                                                          --
-- Copyright (c) 2025 Julien Nadeau Carriere (vedge@csoft.net)              --
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

package body Agar.Combo is

  --
  -- Create a new Combo widget.
  --
  function New_Combo
    (Parent        : in AGW.Widget_Access;
     Text          : in String := "";
     Focusable     : in Boolean := True;
     H_Fill        : in Boolean := False;
     V_Fill        : in Boolean := False;
     Expand        : in Boolean := False;
     Poll          : in Boolean := False;
     Any_Text      : in Boolean := False;
     Scroll_To_Sel : in Boolean := False) return Combo_not_null_Access
  is
    Ch_Text  : aliased C.char_array := C.To_C(Text);
    C_Flags  : aliased C.unsigned := 0;
    Combo    : Combo_Access;
  begin
    if (Poll)          then C_Flags := C_Flags or AG_COMBO_POLL;        end if;
    if (Any_Text)      then C_Flags := C_Flags or AG_COMBO_ANY_TEXT;    end if;
    if (H_Fill)        then C_Flags := C_Flags or AG_COMBO_HFILL;       end if;
    if (V_Fill)        then C_Flags := C_Flags or AG_COMBO_VFILL;       end if;
    if (Expand)        then C_Flags := C_Flags or AG_COMBO_EXPAND;      end if;
    if (Scroll_To_Sel) then C_Flags := C_Flags or AG_COMBO_SCROLLTOSEL; end if;

    if (Text /= "") then
      Combo := AG_ComboNewS
        (Parent => Parent,
         Flags  => C_Flags,
         Text   => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
    else
      Combo := AG_ComboNewS
        (Parent => Parent,
         Flags  => C_Flags,
         Text   => CS.To_Chars_Ptr(null));
    end if;

    if (not Focusable) then
      AGW.Set_Focusable
        (Widget => Combo_To_Widget(Combo),
         Enable => False);
    end if;

    return (Combo);
  end;

  --
  -- Define whether the combo box can be focused.
  --
  procedure Set_Combo_Focusable
    (Combo     : in Combo_not_null_Access;
     Focusable : in Boolean)
  is begin
    AGW.Set_Focusable
      (Widget => Combo_To_Widget(Combo),
       Enable => Focusable);
  end;

  --
  -- Request an initial expanded window size that can contain the given
  -- text and height in pixels. If Height is -1, size automatically.
  --
  procedure Combo_Size_Hint
    (Combo : in Combo_not_null_Access;
     Text  : in String := "<XXXXXXXXXXXXXXX>";
     H     : in Integer := -1)
  is
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    AG_ComboSizeHint
      (Combo => Combo,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access),
       H     => C.int(H));
  end;

  --
  -- Request an initial expanded window size in pixels.
  -- If Width or Height is -1, size automatically.
  --
  procedure Combo_Size_Hint
    (Combo : in Combo_not_null_Access;
     W,H   : in Integer := -1)
  is begin
    AG_ComboSizeHintPixels
      (Combo => Combo,
       W     => C.int(W),
       H     => C.int(H));
  end;

  --
  -- Display the given text in the combo trigger button.
  --
  procedure Set_Combo_Button_Text
    (Combo : in Combo_not_null_Access;
     Text  : in String)
  is
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    AG_ComboSetButtonText
      (Combo => Combo,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

  --
  -- Display the given graphics surface in the combo trigger button.
  -- If Surface is NULL then remove any previously configured surface.
  -- If No_Dup is True then re-use the surface without copy (dangerous).
  --
  procedure Set_Combo_Button_Surface
    (Combo   : in Combo_not_null_Access;
     Surface : in Surface_Access;
     No_Dup  : in Boolean := False)
  is begin
    if (No_Dup) then
      AG_ComboSetButtonSurfaceNODUP
        (Combo   => Combo,
         Surface => Surface);
    else
      AG_ComboSetButtonSurface
        (Combo   => Combo,
         Surface => Surface);
    end if;
  end;

  --
  -- Mark the given item in the list as selected (search by item text).
  --
  function Select_Item
    (Combo : in Combo_not_null_Access;
     Text  : in String) return Agar.Tlist.Tlist_Item_Access
  is
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    return AG_ComboSelectText
      (Combo => Combo,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

end Agar.Combo;
