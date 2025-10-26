------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                           A G A R  . T L I S T                           --
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

package body Agar.Tlist is

  --
  -- Create a new Tlist widget.
  --
  function New_Tlist
    (Parent          : in AGW.Widget_Access;
     Focusable       : in Boolean := True;
     Multi           : in Boolean := False;
     Multi_Toggle    : in Boolean := False;
     Poll            : in Boolean := False;
     Selected_Events : in Boolean := True;
     Scale_Icons     : in Boolean := True;
     H_Fill          : in Boolean := False;
     V_Fill          : in Boolean := False;
     Expand          : in Boolean := False;
     Fixed_Height    : in Boolean := False;
     Stateless       : in Boolean := False;
     Scroll_To_Sel   : in Boolean := False;
     Expand_Nodes    : in Boolean := False;
     Key_Repeat      : in Boolean := True;
     Item_Lines      : in Boolean := True;
     BG_Lines        : in Boolean := True) return Tlist_not_null_Access
  is
    C_Flags  : aliased C.unsigned := 0;
    Tlist    : Tlist_Access;
  begin
    if (Multi)               then C_Flags := C_Flags or AG_TLIST_MULTI;         end if;
    if (Multi_Toggle)        then C_Flags := C_Flags or AG_TLIST_MULTITOGGLE;   end if;
    if (Poll)                then C_Flags := C_Flags or AG_TLIST_POLL;          end if;
    if (not Selected_Events) then C_Flags := C_Flags or AG_TLIST_NO_SELECTED;   end if;
    if (not Scale_Icons)     then C_Flags := C_Flags or AG_TLIST_NO_SCALE_ICON; end if;
    if (H_Fill)              then C_Flags := C_Flags or AG_TLIST_HFILL;         end if;
    if (V_Fill)              then C_Flags := C_Flags or AG_TLIST_VFILL;         end if;
    if (Expand)              then C_Flags := C_Flags or AG_TLIST_EXPAND;        end if;
    if (Fixed_Height)        then C_Flags := C_Flags or AG_TLIST_FIXED_HEIGHT;  end if;
    if (Stateless)           then C_Flags := C_Flags or AG_TLIST_STATELESS;     end if;
    if (Scroll_To_Sel)       then C_Flags := C_Flags or AG_TLIST_SCROLLTOSEL;   end if;
    if (Expand_Nodes)        then C_Flags := C_Flags or AG_TLIST_EXPAND_NODES;  end if;
    if (not Key_Repeat)      then C_Flags := C_Flags or AG_TLIST_NO_KEYREPEAT;  end if;
    if (not Item_Lines)      then C_Flags := C_Flags or AG_TLIST_NO_LINES;      end if;
    if (not BG_Lines)        then C_Flags := C_Flags or AG_TLIST_NO_BGLINES;    end if;

    Tlist := AG_TlistNew
      (Parent => Parent,
       Flags  => C_Flags);

    if (not Focusable) then
      AGW.Set_Focusable
        (Widget => Tlist_To_Widget(Tlist),
         Enable => False);
    end if;

    return (Tlist);
  end;

  --
  -- Define whether the tlist can be focused.
  --
  procedure Tlist_Set_Focusable
    (Tlist     : in Tlist_not_null_Access;
     Focusable : in Boolean)
  is begin
    AGW.Set_Focusable
      (Widget => Tlist_To_Widget(Tlist),
       Enable => Focusable);
  end;

  --
  -- Request an initial size that can contain the given text and number of items.
  --
  procedure Tlist_Size_Hint
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "<XXXXXXXXXXXXXXX>";
     Count : in Natural)
  is
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    AG_TlistSizeHint
      (Tlist => Tlist,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access),
       Count => C.int(Count));
  end;

  --
  -- Request an initial size in pixels.
  -- If Width is -1, size automatically.
  --
  procedure Tlist_Size_Hint
    (Tlist : in Tlist_not_null_Access;
     W     : in Integer := -1;
     Count : in Natural)
  is begin
    AG_TlistSizeHintPixels
      (Tlist => Tlist,
       W     => C.int(W),
       Count => C.int(Count));
  end;

  --
  -- Request an initial size that can contain the largest text label in the
  -- current list of items, and the given item count.
  --
  procedure Tlist_Size_Hint_Largest
    (Tlist : in Tlist_not_null_Access;
     Count : in Natural)
  is begin
    AG_TlistSizeHintLargest
      (Tlist => Tlist,
       Count => C.int(Count));
  end;

  --
  -- Set the height of items in pixels.
  --
  procedure Tlist_Set_Item_Height
    (Tlist  : in Tlist_not_null_Access;
     Item_H : in Natural)
  is begin
    AG_TlistSetItemHeight
      (Tlist  => Tlist,
       Item_H => C.int(Item_H));
  end;

  --
  -- Set the width of icons in pixels.
  --
  procedure Tlist_Set_Icon_Width
    (Tlist  : in Tlist_not_null_Access;
     Icon_W : in Natural)
  is begin
    AG_TlistSetIconWidth
      (Tlist  => Tlist,
       Icon_W => C.int(Icon_W));
  end;

  --
  -- Set the refresh rate for Polled mode in milliseconds (-1 = disable).
  --
  procedure Tlist_Set_Refresh
    (Tlist : in Tlist_not_null_Access;
     Rate  : in Integer := -1)
  is begin
    AG_TlistSetRefresh
      (Tlist => Tlist,
       Rate  => C.int(Rate));      
  end;

  --
  -- In the context of a polling routine, evaluate whether a newly-created
  -- item should make its own child items visible based on the previously
  -- saved state. If there are no items in the saved state which match the
  -- newly-created item (according to the Compare function), then return True
  -- if the Tlist option EXPAND_NODES is set, otherwise return False.
  --
  function Tlist_Visible_Children
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access) return Boolean
  is begin
    return 1 = AG_TlistVisibleChildren
      (Tlist => Tlist,
       Item  => Item);
  end;

  --
  -- Insert an item at the tail of the list and return it.
  --
  function Tlist_Add
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null) return Tlist_Item_not_null_Access
  is
     Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    return AG_TlistAddS
      (Tlist => Tlist,
       Icon  => Icon,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

  --
  -- Insert an item at the tail of the list.
  --
  procedure Tlist_Add
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null)
  is
     Ch_Text : aliased C.char_array := C.To_C(Text);
     Unused  : Tlist_Item_Access;
  begin
    Unused := AG_TlistAddS
      (Tlist => Tlist,
       Icon  => Icon,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

  --
  -- Insert an item at the tail of the list and return it (tagged with a user pointer).
  --
  function Tlist_Add
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null;
     Ptr   : in System.Address) return Tlist_Item_not_null_Access
  is
     Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    return AG_TlistAddPtr
      (Tlist => Tlist,
       Icon  => Icon,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access),
       Ptr   => Ptr);
  end;

  --
  -- Insert an item at the tail of the list (tagged with a user pointer).
  --
  procedure Tlist_Add
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null;
     Ptr   : in System.Address)
  is
     Ch_Text : aliased C.char_array := C.To_C(Text);
     Unused  : Tlist_Item_Access;
  begin
    Unused := AG_TlistAddPtr
      (Tlist => Tlist,
       Icon  => Icon,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access),
       Ptr   => Ptr);
  end;

  --
  -- Insert an item at the head of the list and return it.
  --
  function Tlist_Add_Head
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null) return Tlist_Item_not_null_Access
  is
     Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    return AG_TlistAddHeadS
      (Tlist => Tlist,
       Icon  => Icon,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

  --
  -- Insert an item at the head of the list.
  --
  procedure Tlist_Add_Head
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null)
  is
     Ch_Text : aliased C.char_array := C.To_C(Text);
     Unused  : Tlist_Item_Access;
  begin
    Unused := AG_TlistAddHeadS
      (Tlist => Tlist,
       Icon  => Icon,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

  --
  -- Insert an item at the head of the list and return it (tagged with a user pointer).
  --
  function Tlist_Add_Head
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null;
     Ptr   : in System.Address) return Tlist_Item_not_null_Access
  is
     Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    return AG_TlistAddPtrHead
      (Tlist => Tlist,
       Icon  => Icon,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access),
       Ptr   => Ptr);
  end;

  --
  -- Insert an item at the head of the list (tagged with a user pointer).
  --
  procedure Tlist_Add_Head
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null;
     Ptr   : in System.Address)
  is
     Ch_Text : aliased C.char_array := C.To_C(Text);
     Unused  : Tlist_Item_Access;
  begin
    Unused := AG_TlistAddPtrHead
      (Tlist => Tlist,
       Icon  => Icon,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access),
       Ptr   => Ptr);
  end;

  --
  -- Set an alternate per-item font.
  --
  procedure Tlist_Set_Font
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access;
     Face  : in String := "algue";
     Scale : in C.C_float := C.C_float(1.0);
     Flags : in Agar.Text.Font_Flags := Agar.Text.Font_Flags(0))
  is
    Ch_Face : aliased C.char_array := C.To_C(Face);
  begin
    AG_TlistSetFont
      (Tlist => Tlist,
       Item  => Item,
       Face  => CS.To_Chars_Ptr(Ch_Face'Unchecked_Access),
       Scale => C.C_float(Scale),
       Flags => C.unsigned(Flags));
  end;

  --
  -- Select an item by index.
  --
  procedure Tlist_Select_Item
    (Tlist : in Tlist_not_null_Access;
     Index : in Natural)
  is begin
    AG_TlistSelectIdx
      (Tlist => Tlist,
       Index => C.unsigned(Index));
  end;

  --
  -- Deselect an item by index.
  --
  procedure Tlist_Deselect_Item
    (Tlist : in Tlist_not_null_Access;
     Index : in Natural)
  is begin
    AG_TlistDeselectIdx
      (Tlist => Tlist,
       Index => C.unsigned(Index));
  end;

  --
  -- Select item(s) by text contents.
  --
  function Tlist_Select_By_Text
    (Tlist: in Tlist_not_null_Access;
     Text : in String) return Tlist_Item_Access
  is
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    return AG_TlistSelectText
      (Tlist => Tlist,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

  --
  -- Return the item at the given index (or NULL if no match).
  -- The result is only valid as long as the Tlist is locked.
  --
  function Tlist_Find_By_Index
    (Tlist : in Tlist_not_null_Access;
     Index : in Natural) return Tlist_Item_Access
  is begin
    return AG_TlistFindByIndex
      (Tlist => Tlist,
       Index => C.int(Index));
  end;

  --
  -- Return the first item matching a given text string.
  -- The result is only valid as long as the Tlist is locked.
  --
  function Tlist_Find_By_Text
    (Tlist : in Tlist_not_null_Access;
     Text  : in String) return Tlist_Item_Access
  is
    Ch_Text : aliased C.char_array := C.To_C(Text);
  begin
    return AG_TlistFindText
      (Tlist => Tlist,
       Text  => CS.To_Chars_Ptr(Ch_Text'Unchecked_Access));
  end;

  --
  -- Set a callback to run when the user double clicks on an item.
  -- TODO extra args
  --
  procedure Tlist_Set_Double_Click_Fn
    (Tlist : in Tlist_not_null_Access;
     Fn    : in AGO.Event_Func_Access)
  is begin
    AG_TlistSetDblClickFn
      (Tlist => Tlist,
       Fn    => Fn,
       Args  => CS.Null_Ptr);
  end;

  --
  -- Set a callback to run when the user right-clicks on an item.
  -- TODO extra args
  --
  procedure Tlist_Set_Popup_Fn
    (Tlist : in Tlist_not_null_Access;
     Fn    : in AGO.Event_Func_Access)
  is begin
    AG_TlistSetPopupFn
      (Tlist => Tlist,
       Fn    => Fn,
       Args  => CS.Null_Ptr);
  end;

  --
  -- Set a callback to run when the item selection changes.
  -- TODO extra args
  --
  procedure Tlist_Set_Changed_Fn
    (Tlist : in Tlist_not_null_Access;
     Fn    : in AGO.Event_Func_Access)
  is begin
    AG_TlistSetChangedFn
      (Tlist => Tlist,
       Fn    => Fn,
       Args  => CS.Null_Ptr);
  end;

  --
  -- Set an alternate compare function for items.
  --
  procedure Tlist_Set_Compare_Fn
    (Tlist : in Tlist_not_null_Access;
     Fn    : in Tlist_Compare_Fn_Access)
  is
    Fn_Orig : Tlist_Compare_Fn_Access;
  begin
    Fn_Orig := AG_TlistSetCompareFn
      (Tlist => Tlist,
       Fn    => Fn);
  end;

end Agar.Tlist;
