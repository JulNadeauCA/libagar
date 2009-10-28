with C_String;
with Interfaces.C;

package body Agar.Core.Init is
  package UB_Strings renames Ada.Strings.Unbounded;
  package C          renames Interfaces.C;

  use type C.int;
  use type C.unsigned;

  procedure Get_Version
    (Major   : out Natural;
     Minor   : out Natural;
     Patch   : out Natural;
     Release : out UB_Strings.Unbounded_String)
  is
    Version : aliased Thin.Core.Version_t;
  begin
    Thin.Core.Get_Version (Version'Unchecked_Access);

    Major   := Natural (Version.Major);
    Minor   := Natural (Version.Minor);
    Patch   := Natural (Version.Patch);
    Release := UB_Strings.To_Unbounded_String (C_String.To_String (Version.Release));
  end Get_Version;

  function Init_Flags_To_C_Flags
    (Flags : in Init_Flags_t) return C.unsigned
  is
    C_Flags : C.unsigned := 0;
  begin
    for Index in Init_Flags_t'Range loop
      if Flags (Index) then
        case Index is
          when Verbose               => C_Flags := C_Flags or Thin.Core.AG_VERBOSE;
          when Create_Data_Directory => C_Flags := C_Flags or Thin.Core.AG_CREATE_DATADIR;
          when No_Config_Autoload    => C_Flags := C_Flags or Thin.Core.AG_NO_CFG_AUTOLOAD;
        end case;
      end if;
    end loop;
    return C_Flags;
  end Init_Flags_To_C_Flags;

  function Init_Core
    (Program_Name : in String;
     Flags        : in Init_Flags_t) return Boolean
  is
    Ch_Name : aliased C.char_array := C.To_C (Program_Name);
    C_Flags : constant C.unsigned  := Init_Flags_To_C_Flags (Flags);
  begin
    return 0 = Thin.Core.Init_Core
      (Progname => C_String.To_C_String (Ch_Name'Unchecked_Access),
       Flags    => C_Flags);
  end Init_Core;

--  function Video_Flags_To_C_Flags
--    (Flags : in Video_Flags_t) return C.unsigned
--  is
--    C_Flags : C.unsigned := 0;
--  begin
--    for Index in Video_Flags_t'Range loop
--      if Video_Flags (Index) then
--        case Index in
--          when Video_Hardware_Surface      =>
--          when Video_Asynchronous_Blit     =>
--          when Video_Any_Format            =>
--          when Video_Hardware_Palette      =>
--          when Video_Double_Buffer         =>
--          when Video_Fullscreen            =>
--          when Video_Resizable             =>
--          when Video_No_Frame              =>
--          when Video_Background_Popup_Menu =>
--          when Video_OpenGL                =>
--          when Video_OpenGL_Or_SDL         =>
--          when Video_Overlay               =>
--        end case;
--      end if;
--    end loop;
--    return C_Flags;
--  end Video_Flags_To_C_Flags;

  --
  -- Proxy procedure to call 'Exit_Callback' from C.
  --

  procedure At_Exit_Caller;
  pragma Convention (C, At_Exit_Caller);

  Exit_Callback : Exit_Procedure_t := null;

  procedure At_Exit_Caller is
  begin
    if Exit_Callback /= null then
      Exit_Callback.all;
    end if;
  end At_Exit_Caller;

  procedure At_Exit
    (Callback : Exit_Procedure_Not_Null_t) is
  begin
    Exit_Callback := Callback;
    Thin.Core.At_Exit_Func (At_Exit_Caller'Access);
  end At_Exit;

end Agar.Core.Init;
