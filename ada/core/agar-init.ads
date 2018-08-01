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

  type Agar_Version_t is limited record
    Major : C.int;
    Minor : C.int;
    Patch : C.int;
  end record
    with Convention => C;
  type Agar_Version_Access_t is access all Agar_Version_t with Convention => C;

  procedure Get_Version
    (Major   : out Natural;
     Minor   : out Natural;
     Patch   : out Natural);

  function Init_Core
    (Program_Name     : in String;
     Verbose          : in Boolean := False;
     Create_Directory : in Boolean := False;
     Software_Timers  : in Boolean := False) return Boolean;
  
  function Init_Core
    (Verbose          : in Boolean := False;
     Create_Directory : in Boolean := False;
     Software_Timers  : in Boolean := False) return Boolean;

  type    Exit_Func_Access_t is not null access procedure with Convention => C;
  type    Exit_Procedure_t            is access procedure with Convention => C;
  subtype Exit_Procedure_Not_Null_t is not null Exit_Procedure_t;

  procedure At_Exit (Callback : Exit_Procedure_Not_Null_t);

  procedure Quit
    with Import, Convention => C, Link_Name => "AG_Quit";

  procedure Destroy
    with Import, Convention => C, Link_Name => "AG_Destroy";

  private
  
  use type C.int;
  use type C.unsigned;

  procedure AG_GetVersion (Version : Agar_Version_Access_t)
    with Import, Convention => C, Link_Name => "AG_GetVersion";

  -- XXX TODO typesubst
  AG_VERBOSE		: constant := 16#0001#;
  AG_CREATE_DATADIR	: constant := 16#0002#;
  AG_SOFT_TIMERS	: constant := 16#0004#;

  function AG_InitCore
    (Progname : in CS.chars_ptr;
     Flags    : in C.unsigned) return C.int
    with Import, Convention => C, Link_Name => "AG_InitCore";

  procedure AG_AtExitFunc (Func : in Exit_Func_Access_t)
    with Import, Convention => C, Link_Name => "AG_AtExitFunc";

end Agar.Init;
