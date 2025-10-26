------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                           A G A R  . C O M B O                           --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces.C;
with Interfaces.C.Strings;
with System;
with Agar.Object;
with Agar.Event;
with Agar.Surface; use Agar.Surface;
with Agar.Widget;
with Agar.Button;
with Agar.Tlist;

--
-- Combo box widget. It embeds an AG_Textbox(3) and a button which activates
-- a drop-down menu (an AG_Tlist(3) displayed in a separate window).
--

package Agar.Combo is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;
  package AGO renames Agar.Object;
  package AGW renames Agar.Widget;

  use type C.int;
  use type C.unsigned;

  -- Flags --
  AG_COMBO_POLL        : constant C.unsigned := 16#00_01#;
  AG_COMBO_ANY_TEXT    : constant C.unsigned := 16#00_04#;
  AG_COMBO_HFILL       : constant C.unsigned := 16#00_08#;
  AG_COMBO_VFILL       : constant C.unsigned := 16#00_10#;
  AG_COMBO_SCROLLTOSEL : constant C.unsigned := 16#00_40#;
  AG_COMBO_EXPAND      : constant C.unsigned := AG_COMBO_HFILL or AG_COMBO_VFILL;

  --------------------------------
  -- Agar Combo Widget Instance --
  --------------------------------
  type Combo is limited record
    Super              : aliased Agar.Widget.Widget; -- ( Object -> Widget -> Combo )
    Flags              : C.unsigned;
    Visible_Item_Count : C.unsigned;                 -- Number of visible items
    Input_Textbox      : System.Address;             -- TODO Input textbox
    Contd_Button       : Agar.Button.Button_Access;  -- [...] button
    List               : Agar.Tlist.Tlist_Access;    -- List of items (original)
    Exp_List           : Agar.Tlist.Tlist_Access;    -- List of items (in expanded window)
    Exp_Window         : AGW.Window_Access;          -- Expanded window
    W_Saved, H_Saved   : C.int;                      -- Saved window size
    W_Req, H_Req       : C.int;                      -- Size hint
  end record
    with Convention => C;

  type Combo_Access is access all Combo with Convention => C;
  subtype Combo_not_null_Access is not null Combo_Access;

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
     Scroll_To_Sel : in Boolean := False) return Combo_not_null_Access;

  function Combo_To_Widget
    (Combo : in Combo_not_null_Access) return AGW.Widget_not_null_Access
    with Import, Convention => C, Link_Name => "ag_combo_to_widget";

  function Combo_To_Object
    (Combo : in Combo_not_null_Access) return AGO.Object_not_null_Access
    with Import, Convention => C, Link_Name => "ag_combo_to_object";

  --
  -- Define whether the combo can be focused.
  --
  procedure Set_Combo_Focusable
    (Combo     : in Combo_not_null_Access;
     Focusable : in Boolean);

  --
  -- Request an initial expanded window size that can contain the given
  -- text and height in pixels. If Height is -1, size automatically.
  --
  procedure Combo_Size_Hint
    (Combo : in Combo_not_null_Access;
     Text  : in String := "<XXXXXXXXXXXXXXX>";
     H     : in Integer := -1);

  --
  -- Request an initial expanded window size in pixels.
  -- If Width or Height is -1, size automatically.
  --
  procedure Combo_Size_Hint
    (Combo : in Combo_not_null_Access;
     W,H   : in Integer := -1);

  --
  -- Display the given text in the combo trigger button.
  --
  procedure Set_Combo_Button_Text
    (Combo : in Combo_not_null_Access;
     Text  : in String);

  --
  -- Display the given graphics surface in the combo trigger button.
  -- If Surface is NULL then remove any previously configured surface.
  -- If No_Dup is True then re-use the surface without copy (dangerous).
  --
  procedure Set_Combo_Button_Surface
    (Combo   : in Combo_not_null_Access;
     Surface : in Surface_Access;
     No_Dup  : in Boolean := False);

  --
  -- Mark the given item in the list as selected.
  --
  procedure Select_Item
    (Combo : in Combo_not_null_Access;
     Item  : in Agar.Tlist.Tlist_Item_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ComboSelect"; 

  --
  -- Mark the given item in the list as selected (search by item user pointer).
  --
  function Select_Item
    (Combo   : in Combo_not_null_Access;
     Pointer : in System.Address) return Agar.Tlist.Tlist_Item_Access
    with Import, Convention => C, Link_Name => "AG_ComboSelectPointer"; 

  --
  -- Mark the given item in the list as selected (search by item text).
  --
  function Select_Item
    (Combo : in Combo_not_null_Access;
     Text  : in String) return Agar.Tlist.Tlist_Item_Access;

  private

  function AG_ComboNewS
    (Parent : in AGW.Widget_Access;
     Flags  : in C.unsigned;
     Text   : in CS.chars_ptr) return Combo_not_null_Access
    with Import, Convention => C, Link_Name => "AG_ComboNewS";

  procedure AG_ComboSizeHint
    (Combo : in Combo_not_null_Access;
     Text  : in CS.chars_ptr;
     H     : in C.int)
    with Import, Convention => C, Link_Name => "AG_ComboSizeHint";

  procedure AG_ComboSizeHintPixels
    (Combo : in Combo_not_null_Access;
     W,H   : in C.int)
    with Import, Convention => C, Link_Name => "AG_ComboSizeHintPixels";

  function AG_ComboSelectText
    (Combo : in Combo_not_null_Access;
     Text  : in CS.chars_ptr) return Agar.Tlist.Tlist_Item_Access
    with Import, Convention => C, Link_Name => "AG_ComboSelectText";

  procedure AG_ComboSetButtonText
    (Combo : in Combo_not_null_Access;
     Text  : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_ComboSetButtonText";

  procedure AG_ComboSetButtonSurface
    (Combo   : in Combo_not_null_Access;
     Surface : in Surface_Access)
    with Import, Convention => C, Link_Name => "AG_ComboSetButtonSurface";

  procedure AG_ComboSetButtonSurfaceNODUP
    (Combo   : in Combo_not_null_Access;
     Surface : in Surface_Access)
    with Import, Convention => C, Link_Name => "AG_ComboSetButtonSurfaceNODUP";

end Agar.Combo;
