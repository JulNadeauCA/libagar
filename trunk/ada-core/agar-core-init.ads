with Agar.Core.Thin;
with Ada.Strings.Unbounded;

package Agar.Core.Init is

  --
  -- Get_Version.
  --

  procedure Get_Version
    (Major   : out Natural;
     Minor   : out Natural;
     Patch   : out Natural;
     Release : out Ada.Strings.Unbounded.Unbounded_String);

  type Init_Flags_Select_t is
    (Verbose,
     Create_Data_Directory,
     No_Config_Autoload);
  type Init_Flags_t is array (Init_Flags_Select_t) of Boolean;

  --
  -- Init_Core.
  --

  function Init_Core
    (Program_Name : in String;
     Flags        : in Init_Flags_t) return Boolean;

  type Video_Flags_Select_t is
    (Video_Hardware_Surface,
     Video_Asynchronous_Blit,
     Video_Any_Format,
     Video_Hardware_Palette,
     Video_Double_Buffer,
     Video_Fullscreen,
     Video_Resizable,
     Video_No_Frame,
     Video_Background_Popup_Menu,
     Video_OpenGL,
     Video_OpenGL_Or_SDL,
     Video_Overlay);
  type Video_Flags_t is array (Video_Flags_Select_t) of Boolean;

  --
  -- At_Exit.
  --

  type    Exit_Procedure_t          is access procedure;
  subtype Exit_Procedure_Not_Null_t is not null Exit_Procedure_t;

  procedure At_Exit (Callback : Exit_Procedure_Not_Null_t);

  --
  -- Quit.
  --

  procedure Quit renames Thin.Core.Quit;

  --
  -- Destroy.
  --

  procedure Destroy renames Thin.Core.Destroy;

end Agar.Core.Init;
