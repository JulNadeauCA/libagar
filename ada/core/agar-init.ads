------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                            A G A R . I N I T                             --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces.C;
with Interfaces.C.Strings;

package Agar.Init is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;

  type Agar_Version is limited record
    Major    : C.int;                  -- Major version number
    Minor    : C.int;                  -- Minor version number
    Patch    : C.int;                  -- Patchlevel
    Revision : C.int;                  -- SVN revision number (or 0)
    Release  : CS.chars_ptr;           -- Release name (or NULL)
  end record
    with Convention => C;
  type Agar_Version_Access is access all Agar_Version with Convention => C;
  
  type Atexit_Func_Access is access procedure with Convention => C;

  --
  -- Get the Agar version number.
  --
  procedure Get_Version
    (Major : out Natural;
     Minor : out Natural;
     Patch : out Natural);

  --
  -- Initialize the Agar-Core library.
  --
  function Init_Core
    (Program_Name     : in String;
     Verbose          : in Boolean := False;
     Create_Directory : in Boolean := False;
     Software_Timers  : in Boolean := False) return Boolean;
  
  function Init_Core
    (Verbose          : in Boolean := False;
     Create_Directory : in Boolean := False;
     Software_Timers  : in Boolean := False) return Boolean;

  --
  -- Set an exit callback routine.
  --
  procedure At_Exit (Callback : Atexit_Func_Access);

  --
  -- Release all resources and exit application.
  --
  procedure Quit
    with Import, Convention => C, Link_Name => "AG_Quit";

  --
  -- Release all resources allocated by Agar-Core.
  --
  procedure Destroy
    with Import, Convention => C, Link_Name => "AG_Destroy";

  private
  
  use type C.int;
  use type C.unsigned;

  procedure AG_GetVersion (Version : Agar_Version_Access)
    with Import, Convention => C, Link_Name => "AG_GetVersion";

  AG_VERBOSE		: constant := 16#01#;    -- Verbose() to console
  AG_CREATE_DATADIR	: constant := 16#02#;    -- Check and create data dir
  AG_SOFT_TIMERS	: constant := 16#04#;    -- Force software timing wheel

  function AG_InitCore
    (Progname : in CS.chars_ptr;
     Flags    : in C.unsigned) return C.int
    with Import, Convention => C, Link_Name => "AG_InitCore";

  procedure AG_AtExitFunc (Func : in Atexit_Func_Access)
    with Import, Convention => C, Link_Name => "AG_AtExitFunc";

end Agar.Init;
