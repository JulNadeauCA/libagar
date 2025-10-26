------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                           A G A R  . T L I S T                           --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces; use Interfaces;
with Interfaces.C;
with Interfaces.C.Strings;
with System;
with Agar.Types; use Agar.Types;
with Agar.Object;
with Agar.Event;
with Agar.Surface; use Agar.Surface;
with Agar.Widget;
with Agar.Button;
with Agar.Text;
with Agar.Timer;

--
-- Tree/List View widget. It shows a scrollable tree (or list) of clickable
-- and selectable text items. It provides a polling mode with asset-recycling
-- for lists which must be cleared and repopulated frequently.
--

package Agar.Tlist is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;
  package AGO renames Agar.Object;
  package AGW renames Agar.Widget;

  use type C.int;
  use type C.unsigned;
  use type C.C_float;

  -- Limits --
  TLIST_LABEL_MAX : constant Natural := $AG_TLIST_LABEL_MAX;

  -- Flags --
  AG_TLIST_MULTI         : constant C.unsigned := 16#00_01#;     -- Multiple selections (ctrl/shift)
  AG_TLIST_MULTITOGGLE   : constant C.unsigned := 16#00_02#;     -- Multiple toggle-style selections
  AG_TLIST_POLL          : constant C.unsigned := 16#00_04#;     -- Generate "tlist-poll" events
  AG_TLIST_NO_SELECTED   : constant C.unsigned := 16#00_08#;     -- Inhibit "tlist-selected" event
  AG_TLIST_NO_SCALE_ICON : constant C.unsigned := 16#00_10#;     -- Don't scale oversize icons
  AG_TLIST_HFILL         : constant C.unsigned := 16#00_20#;
  AG_TLIST_VFILL         : constant C.unsigned := 16#00_40#;
  AG_TLIST_FIXED_HEIGHT  : constant C.unsigned := 16#00_80#;     -- Don't set icon height on "font-changed"
  AG_TLIST_STATELESS     : constant C.unsigned := 16#01_00#;     -- Don't preserve selection state (POLL mode)
  AG_TLIST_SCROLLTOSEL   : constant C.unsigned := 16#02_00#;     -- Scroll to initial selection
  AG_TLIST_REFRESH       : constant C.unsigned := 16#04_00#;     -- Repopulate now (POLL mode)
  AG_TLIST_EXPAND_NODES  : constant C.unsigned := 16#08_00#;     -- Expand node items (items with children) by default
  AG_TLIST_NO_KEYREPEAT  : constant C.unsigned := 16#10_00#;     -- Disable keyrepeat behavior
  AG_TLIST_NO_LINES      : constant C.unsigned := 16#20_00#;     -- Don't draw lines connecting items
  AG_TLIST_NO_BGLINES    : constant C.unsigned := 16#40_00#;     -- Don't draw lines in background
  AG_TLIST_EXPAND        : constant C.unsigned := AG_TLIST_HFILL or AG_TLIST_VFILL;

  type Tlist_Item;
  type Tlist_Item_Access is access all Tlist_Item with Convention => C;
  subtype Tlist_Item_not_null_Access is not null Tlist_Item_Access;

  type Tlist_Item_Entry is limited record
    Next : Tlist_Item_Access;
    Prev : access Tlist_Item_Access;
  end record
    with Convention => C;

  type Tlist_Item_Text_Label is array (1 .. TLIST_LABEL_MAX)
    of aliased C.char with Convention => C;

  ---------------------
  -- Agar Tlist Item --
  ---------------------
  type Tlist_Item is limited record
    Tag                : Unsigned_32;           -- Tagged non-object
    Label_Disabled     : C.int;                 -- Rendered surface (while #disabled)
    Label_Enabled      : C.int;                 -- Rendered surface (while #enabled)
    Label_Selected     : C.int;                 -- Rendered surface (while #selected)
    V                  : C.int;                 -- App-specific integer / sort key
    Category           : CS.chars_ptr;          -- App-specific category ID string
    Icon_Source        : Surface_Access;        -- Icon source image
    Ptr                : System.Address;        -- App-specific user pointer
    Color              : AG_Color_Access;       -- Alternate color for this item
    Font               : System.Address;        -- Alternate font for this item (TODO)
    Selected           : C.int;                 -- Selection flag
    Depth              : C.unsigned;            -- Indent in tree display
    Flags              : C.unsigned;
    Scale              : C.C_float;             -- Text scaling factor (default = 1.0)
    Text               : Tlist_Item_Text_Label; -- Text label contents
    U                  : C.unsigned;            -- App-specific unsigned integer
    Entry_in_Items     : Tlist_Item_Entry;      -- Items in the Tlist
    Entry_in_Sel_Items : Tlist_Item_Entry;      -- Saved selection state
  end record
    with Convention => C;

  type Tlist_Expansion_Levels is access all C.int with Convention => C;

  type Tlist_Popup_Menu;
  type Tlist_Popup_Menu_Access is access all Tlist_Popup_Menu with Convention => C;
  subtype Tlist_Popup_Menu_not_null_Access is not null Tlist_Popup_Menu_Access;

  type Tlist_Popup_Menu_Entry is limited record
    Next : Tlist_Popup_Menu_Access;
    Prev : access Tlist_Popup_Menu_Access;
  end record
    with Convention => C;

  ---------------------------
  -- Agar Tlist Popup Menu --
  ---------------------------
  type Tlist_Popup_Menu is limited record
    Class_Filter         : CS.chars_ptr;                         -- Apply to items of this category
--    Menu                 : Agar.Menu.Menu_not_null_Access;       -- The popup menu proper
--    Item                 : Agar.Menu.Menu_not_null_Item;         -- Root item of the popup menu
    Menu                 : System.Address; -- TODO
    Item                 : System.Address;
    Panel                : AGW.Window_Access;                     -- Generated window expansion
    Entry_in_Popup_Menus : Tlist_Popup_Menu_Entry;
  end record
    with Convention => C;

  type Tlist_Popup_Menu_Q is limited record
    First : Tlist_Popup_Menu_Access;
    Last  : access Tlist_Popup_Menu_Access;
  end record
    with Convention => C;

  type Tlist_Compare_Fn_Access is access function
    (A,B : Tlist_Item_not_null_Access) return C.int
    with Convention => C;

  type Tlist_BG_Line_Colors is array (1 .. AGW.WIDGET_NSTATES)
    of aliased AG_Color with Convention => C;

  --------------------------------
  -- Agar Tlist Widget Instance --
  --------------------------------
  type Tlist is limited record
    Super              : aliased Agar.Widget.Widget;      -- ( Object -> Widget -> Tlist )
    Flags              : C.unsigned;
    Item_H             : C.int;                           -- Item height
    Selected_Ptr       : System.Address;                  -- Default "selected" binding
    W_Hint, H_Hint     : C.int;                           -- Size hint
    Rect               : AG_Rect;                         -- Clipping rectangle
    Exp_Levels         : Tlist_Expansion_Levels;          -- Tree expansion state for draw()
    Exp_Levels_Count   : C.unsigned;
    Icon_W             : C.int;                           -- Item icon width
    Poll_Delay         : C.unsigned;                      -- Refresh rate (for POLL mode)
    Row_Offset         : C.int;                           -- Row display offset
    Double_Clicked_Ptr : System.Address;                  -- For double click test
    Items              : Tlist_Item_Entry;                -- Current items
    Sel_Items          : Tlist_Item_Entry;                -- Saved item states
    Items_Count        : C.int;                           -- Number of items in total
    Visible_Count      : C.int;                           -- Number of items on screen
--    V_Bar              : Agar.Scrollbar.Scrollbar_Access; -- Vertical scrollbar
    V_Bar              : System.Address; -- TODO
    Popup_Menus        : Tlist_Popup_Menu_Q;              -- Popup menus
    Compare_Fn         : Tlist_Compare_Fn_Access;         -- Item-item comparison function
    Popup_Event        : Agar.Event.Event_Access;         -- Popup menu hook
    Changed_Event      : Agar.Event.Event_Access;         -- Selection change hook
    Double_Click_Event : Agar.Event.Event_Access;         -- Double click hook
    Line_Scroll_Amount : C.int;
    Last_KeyDown       : C.int;                           -- For key repeat
    Move_Timer         : Agar.Timer.Timer;                -- For keyboard motion
    Refresh_Timer      : Agar.Timer.Timer;                -- For polled mode updates
    Double_Click_Timer : Agar.Timer.Timer;                -- For detecting double clicks
    Ctrl_Move_Timer    : Agar.Timer.Timer;                -- For controller-drive move
    Color_BG_Line      : Tlist_BG_Line_Colors;            -- Background line color (stateful)
  end record
    with Convention => C;

  type Tlist_Access is access all Tlist with Convention => C;
  subtype Tlist_not_null_Access is not null Tlist_Access;

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
     BG_Lines        : in Boolean := True) return Tlist_not_null_Access;

  function Tlist_To_Widget
    (Tlist : in Tlist_not_null_Access) return AGW.Widget_not_null_Access
    with Import, Convention => C, Link_Name => "ag_tlist_to_widget";

  function Tlist_To_Object
    (Tlist : in Tlist_not_null_Access) return AGO.Object_not_null_Access
    with Import, Convention => C, Link_Name => "ag_tlist_to_object";

  --
  -- Define whether the tlist can be focused.
  --
  procedure Tlist_Set_Focusable
    (Tlist     : in Tlist_not_null_Access;
     Focusable : in Boolean);

  --
  -- Request an initial size that can contain the given text and number of items.
  --
  procedure Tlist_Size_Hint
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "<XXXXXXXXXXXXXXX>";
     Count : in Natural);

  --
  -- Request an initial size in pixels.
  -- If Width is -1, size automatically.
  --
  procedure Tlist_Size_Hint
    (Tlist : in Tlist_not_null_Access;
     W     : in Integer := -1;
     Count : in Natural);

  --
  -- Request an initial size that can contain the largest text label in the
  -- current list of items, and the given item count.
  --
  procedure Tlist_Size_Hint_Largest
    (Tlist : in Tlist_not_null_Access;
     Count : in Natural);

  --
  -- Set the height of items in pixels.
  --
  procedure Tlist_Set_Item_Height
    (Tlist  : in Tlist_not_null_Access;
     Item_H : in Natural);

  --
  -- Set the width of icons in pixels.
  --
  procedure Tlist_Set_Icon_Width
    (Tlist  : in Tlist_not_null_Access;
     Icon_W : in Natural);

  --
  -- Set the refresh rate for Polled mode in milliseconds (-1 = disable).
  --
  procedure Tlist_Set_Refresh
    (Tlist : in Tlist_not_null_Access;
     Rate  : in Integer := -1);

  --
  -- Remove duplicate items from the list.
  --
  procedure Tlist_Uniq
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistUniq";

  --
  -- Begin populating the list in a POLL mode function.
  --
  procedure Tlist_Begin
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistBegin";

  --
  -- Finalize populating the list in a POLL mode function.
  --
  procedure Tlist_End
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistEnd";

  --
  -- In the context of a polling routine, evaluate whether a newly-created
  -- item should make its own child items visible based on the previously
  -- saved state. If there are no items in the saved state which match the
  -- newly-created item (according to the Compare function), then return True
  -- if the Tlist option EXPAND_NODES is set, otherwise return False.
  --
  function Tlist_Visible_Children
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access) return Boolean;

  --
  -- Clear items on the list. Same operation as Tlist_Begin.
  -- If POLL mode is in effect, save the selection state.
  --
  procedure Tlist_Clear
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistBegin";

  --
  -- Restore previous item selection state. Same operation as Tlist_end.
  --
  procedure Tlist_Restore
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistEnd";

  --
  -- Insert an item at the tail of the list and return it.
  --
  function Tlist_Add
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null) return Tlist_Item_not_null_Access;

  --
  -- Insert an item at the tail of the list.
  --
  procedure Tlist_Add
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null);

  --
  -- Insert an item at the tail of the list and return it (tagged with user pointer).
  --
  function Tlist_Add
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null;
     Ptr   : in System.Address) return Tlist_Item_not_null_Access;

  --
  -- Insert an item at the tail of the list (tagged with a user pointer).
  --
  procedure Tlist_Add
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null;
     Ptr   : in System.Address);

  --
  -- Insert an item at the head of the list and return it.
  --
  function Tlist_Add_Head
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null) return Tlist_Item_not_null_Access;

  --
  -- Insert an item at the head of the list.
  --
  procedure Tlist_Add_Head
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null);

  --
  -- Insert an item at the head of the list and return it (tagged with a user pointer).
  --
  function Tlist_Add_Head
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null;
     Ptr   : in System.Address) return Tlist_Item_not_null_Access;

  --
  -- Insert an item at the head of the list (tagged with a user pointer).
  --
  procedure Tlist_Add_Head
    (Tlist : in Tlist_not_null_Access;
     Text  : in String := "";
     Icon  : in Surface_Access := null;
     Ptr   : in System.Address);

  --
  -- Move the given item to the head of the list.
  --
  procedure Tlist_Move_To_Head
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistMoveToHead";

  --
  -- Move the given item to the tail of the list.
  --
  procedure Tlist_Move_To_Tail
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistMoveToTail";

  --
  -- Allocate and initialize a Tlist_Item structure.
  --
  function Tlist_Item_New
    (Icon : in Surface_Access) return Tlist_Item_not_null_Access
    with Import, Convention => C, Link_Name => "AG_TlistItemNew";

  --
  -- Set the graphical icon to display along an item.
  --
  procedure Tlist_Set_Icon
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access;
     Icon  : in Surface_Access := null)
    with Import, Convention => C, Link_Name => "AG_TlistSetIcon";

  --
  -- Set an alternate per-item text color.
  --
  procedure Tlist_Set_Color
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access;
     Color : in AG_Color_Access := null)
    with Import, Convention => C, Link_Name => "AG_TlistSetColor";

  --
  -- Set an alternate per-item font.
  --
  procedure Tlist_Set_Font
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access;
     Face  : in String := "algue";
     Scale : in C.C_float := C.C_float(1.0);
     Flags : in Agar.Text.Font_Flags := Agar.Text.Font_Flags(0));

  --
  -- Delete an item.
  --
  procedure Tlist_Delete
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistDel";  

  --
  -- Select an item by reference.
  --
  procedure Tlist_Select_Item
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistSelect";  

  --
  -- Deselect an item by reference.
  --
  procedure Tlist_Deselect_Item
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistDeselect";  

  --
  -- Select an item by index.
  --
  procedure Tlist_Select_Item
    (Tlist : in Tlist_not_null_Access;
     Index : in Natural);

  --
  -- Deselect an item by index.
  --
  procedure Tlist_Deselect_Item
    (Tlist : in Tlist_not_null_Access;
     Index : in Natural);

  --
  -- Select all items.
  --
  procedure Tlist_Select_All
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistSelectAll";

  --
  -- Deselect all items.
  --
  procedure Tlist_Deselect_All
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistDeselectAll";

  --
  -- Select item(s) by pointer address.
  --
  function Tlist_Select_By_Pointer
    (Tlist   : in Tlist_not_null_Access;
     Pointer : in System.Address) return Tlist_Item_Access
    with Import, Convention => C, Link_Name => "AG_TlistSelectPtr";

  --
  -- Select item(s) by text contents.
  --
  function Tlist_Select_By_Text
    (Tlist : in Tlist_not_null_Access;
     Text  : in String) return Tlist_Item_Access;

  --
  -- Return the item at the given index (or NULL if no match).
  -- The result is only valid as long as the Tlist is locked.
  --
  function Tlist_Find_By_Index
    (Tlist : in Tlist_not_null_Access;
     Index : in Natural) return Tlist_Item_Access;

  --
  -- Return the first selected item (or NULL if no item is selected).
  -- The result is only valid as long as the Tlist is locked.
  --
  function Tlist_Selected_Item
    (Tlist : in Tlist_not_null_Access) return Tlist_Item_Access
    with Import, Convention => C, Link_Name => "AG_TlistSelectedItem";

  --
  -- Return the user pointer of the first selected item (or NULL if no item is selected).
  -- The result is only valid as long as the Tlist is locked.
  --
  function Tlist_Selected_Item_Pointer
    (Tlist : in Tlist_not_null_Access) return System.Address
    with Import, Convention => C, Link_Name => "AG_TlistSelectedItemPtr";

  --
  -- Return the first item on the list (or NULL if the list is empty).
  -- The result is only valid as long as the Tlist is locked.
  --
  function Tlist_First_Item
    (Tlist : in Tlist_not_null_Access) return Tlist_Item_Access
    with Import, Convention => C, Link_Name => "AG_TlistFirstItem";

  --
  -- Return the last item on the list (or NULL if the list is empty).
  -- The result is only valid as long as the Tlist is locked.
  --
  function Tlist_Last_Item
    (Tlist : in Tlist_not_null_Access) return Tlist_Item_Access
    with Import, Convention => C, Link_Name => "AG_TlistLastItem";

  --
  -- Return the user pointer associated with the first selected item (or NULL).
  -- The result is only valid as long as the Tlist is locked.
  --
  function Tlist_First_Item_Pointer
    (Tlist : in Tlist_not_null_Access) return System.Address
    with Import, Convention => C, Link_Name => "AG_TlistFindPtr";

  --
  -- Return the first item matching a given text string.
  -- The result is only valid as long as the Tlist is locked.
  --
  function Tlist_Find_By_Text
    (Tlist : in Tlist_not_null_Access;
     Text  : in String) return Tlist_Item_Access;

  --
  -- TODO Create and return a new popup menu for items of the given category.
  --
  -- function Tlist_Set_Popup_Menu
  --  (Tlist : in Tlist_not_null_Access;
  --   Class : in String) return Menu_Item_not_null_Access;

  --
  -- Scroll to the first selected item (or no-op if there is no selection).
  --
  procedure Tlist_Scroll_To_Selection
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistScrollToSelection";

  --
  -- Scroll to the beginning of the list.
  --
  procedure Tlist_Scroll_To_Start
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistScrollToStart";

  --
  -- Scroll to the end of the list.
  --
  procedure Tlist_Scroll_To_End
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistScrollToEnd";

  --
  -- Set a callback to run when the user double clicks on an item.
  -- TODO extra args
  --
  procedure Tlist_Set_Double_Click_Fn
    (Tlist : in Tlist_not_null_Access;
     Fn    : in AGO.Event_Func_Access);

  --
  -- Set a callback to run when the user right-clicks on an item.
  -- TODO extra args
  --
  procedure Tlist_Set_Popup_Fn
    (Tlist : in Tlist_not_null_Access;
     Fn    : in AGO.Event_Func_Access);

  --
  -- Set a callback to run when the item selection changes.
  -- TODO extra args
  --
  procedure Tlist_Set_Changed_Fn
    (Tlist : in Tlist_not_null_Access;
     Fn    : in AGO.Event_Func_Access);

  --
  -- Set an alternate compare function for items.
  --
  procedure Tlist_Set_Compare_Fn
    (Tlist : in Tlist_not_null_Access;
     Fn    : in Tlist_Compare_Fn_Access);

  --
  -- Compare items by their integer values v (ascending).
  --
  function Tlist_Compare_Ints
    (A,B : in Tlist_Item_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_TlistCompareInts";

  --
  -- Compare items by their integer values v (descending).
  --
  function Tlist_Compare_Ints_Descending
    (A,B : in Tlist_Item_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_TlistCompareIntsDsc";
    
  --
  -- Compare items by their unsigned integer values u (ascending).
  --
  function Tlist_Compare_Uints
    (A,B : in Tlist_Item_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_TlistCompareUints";

  --
  -- Compare items by their text fields (case-sensitively; ignoring locale).
  --
  function Tlist_Compare_Strings
    (A,B : in Tlist_Item_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_TlistCompareStrings";

  --
  -- Compare items by their user pointer values.
  --
  function Tlist_Compare_Pointers
    (A,B : in Tlist_Item_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_TlistComparePtrs";

  --
  -- Compare items by both their user pointer values and categories.
  --
  function Tlist_Compare_Pointers_And_Categories
    (A,B : in Tlist_Item_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_TlistComparePtrsAndCats";

  --
  -- Sort items by their text field using quicksort.
  --
  procedure Tlist_Sort
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistSort";

  --
  -- Sort items by their integer values v using quicksort.
  --
  procedure Tlist_Sort_By_Int
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistSortByInt";

  --
  -- Refresh and redraw the list.
  --
  procedure Tlist_Refresh
    (Tlist : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistRefresh";

  --
  -- Copy the contents of a tlist onto another.
  --
  procedure Tlist_Copy
    (Src, Dest : in Tlist_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TlistCopy";

  private

  function AG_TlistNew
    (Parent : in AGW.Widget_Access;
     Flags  : in C.unsigned) return Tlist_not_null_Access
    with Import, Convention => C, Link_Name => "AG_TlistNew";

  procedure AG_TlistSizeHint
    (Tlist : in Tlist_not_null_Access;
     Text  : in CS.chars_ptr;
     Count : in C.int)
    with Import, Convention => C, Link_Name => "AG_TlistSizeHint";

  procedure AG_TlistSizeHintPixels
    (Tlist    : in Tlist_not_null_Access;
     W, Count : in C.int)
    with Import, Convention => C, Link_Name => "AG_TlistSizeHintPixels";

  procedure AG_TlistSizeHintLargest
    (Tlist : in Tlist_not_null_Access;
     Count : in C.int)
    with Import, Convention => C, Link_Name => "AG_TlistSizeHintLargest";

  procedure AG_TlistSetItemHeight
    (Tlist  : in Tlist_not_null_Access;
     Item_H : in C.int)
    with Import, Convention => C, Link_Name => "AG_TlistSetItemHeight";

  procedure AG_TlistSetIconWidth
    (Tlist : in Tlist_not_null_Access;
     Icon_W : in C.int)
    with Import, Convention => C, Link_Name => "AG_TlistSetIconWidth";

  procedure AG_TlistSetRefresh
    (Tlist : in Tlist_not_null_Access;
     Rate  : in C.int)
    with Import, Convention => C, Link_Name => "AG_TlistSetRefresh";

  function AG_TlistVisibleChildren
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_TlistVisibleChildren";

  procedure AG_TlistSetFont
    (Tlist : in Tlist_not_null_Access;
     Item  : in Tlist_Item_not_null_Access;
     Face  : in CS.chars_ptr;
     Scale : in C.C_float;
     Flags : in C.unsigned)
    with Import, Convention => C, Link_Name => "AG_TlistSetFont";

  procedure AG_TlistSelectIdx
    (Tlist : in Tlist_not_null_Access;
     Index : in C.unsigned)
    with Import, Convention => C, Link_Name => "AG_TlistSelectIdx";

  procedure AG_TlistDeselectIdx
    (Tlist : in Tlist_not_null_Access;
     Index : in C.unsigned)
    with Import, Convention => C, Link_Name => "AG_TlistDeselectIdx";

  function AG_TlistSelectText
    (Tlist : in Tlist_not_null_Access;
     Text  : in CS.chars_ptr) return Tlist_Item_not_null_Access
    with Import, Convention => C, Link_Name => "AG_TlistSelectText";

  function AG_TlistAddS
    (Tlist : in Tlist_not_null_Access;
     Icon  : in Surface_Access;
     Text  : in CS.chars_ptr) return Tlist_Item_not_null_Access
    with Import, Convention => C, Link_Name => "AG_TlistAddS";

  function AG_TlistAddHeadS
    (Tlist : in Tlist_not_null_Access;
     Icon  : in Surface_Access;
     Text  : in CS.chars_ptr) return Tlist_Item_not_null_Access
    with Import, Convention => C, Link_Name => "AG_TlistAddHeadS";

  function AG_TlistAddPtr
    (Tlist : in Tlist_not_null_Access;
     Icon  : in Surface_Access;
     Text  : in CS.chars_ptr;
     Ptr   : in System.Address) return Tlist_Item_not_null_Access
    with Import, Convention => C, Link_Name => "AG_TlistAddPtr";

  function AG_TlistAddPtrHead
    (Tlist : in Tlist_not_null_Access;
     Icon  : in Surface_Access;
     Text  : in CS.chars_ptr;
     Ptr   : in System.Address) return Tlist_Item_not_null_Access
    with Import, Convention => C, Link_Name => "AG_TlistAddPtrHead";

  function AG_TlistFindByIndex
    (Tlist : in Tlist_not_null_Access;
     Index : in C.int) return Tlist_Item_Access
    with Import, Convention => C, Link_Name => "AG_TlistFindByIndex";

  function AG_TlistFindText
    (Tlist : in Tlist_not_null_Access;
     Text  : in CS.chars_ptr) return Tlist_Item_Access
    with Import, Convention => C, Link_Name => "AG_TlistFindText";

  procedure AG_TlistSetDblClickFn
    (Tlist : in Tlist_not_null_Access;
     Fn    : in AGO.Event_Func_Access;
     Args  : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_TlistSetDblClickFn";

  procedure AG_TlistSetPopupFn
    (Tlist : in Tlist_not_null_Access;
     Fn    : in AGO.Event_Func_Access;
     Args  : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_TlistSetPopupFn";

  procedure AG_TlistSetChangedFn
    (Tlist : in Tlist_not_null_Access;
     Fn    : in AGO.Event_Func_Access;
     Args  : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_TlistSetChangedFn";

  function AG_TlistSetCompareFn
    (Tlist : in Tlist_not_null_Access;
     Fn    : in Tlist_Compare_Fn_Access) return Tlist_Compare_Fn_Access
    with Import, Convention => C, Link_Name => "AG_TlistSetCompareFn";

end Agar.Tlist;
