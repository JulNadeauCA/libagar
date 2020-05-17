------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                         A G A R . I N I T _ G U I                        --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces.C;
with Interfaces.C.Strings;

package Agar.Init_GUI is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;

  function Init_Graphics (Driver : in String) return Boolean;
  function Init_GUI return Boolean;

  procedure Destroy_Graphics
    with Import, Convention => C, Link_Name => "AG_DestroyGraphics";
  procedure Destroy_GUI
    with Import, Convention => C, Link_Name => "AG_DestroyGUI";

  private

  use type C.int;

  function AG_InitGraphics (Driver : in CS.chars_ptr) return C.int
    with Import, Convention => C, Link_Name => "AG_InitGraphics";
  
  function AG_InitGUI (Flags : in C.unsigned) return C.int
    with Import, Convention => C, Link_Name => "AG_InitGUI";

end Agar.Init_GUI;
