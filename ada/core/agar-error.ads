------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                           A G A R . E R R O R                            --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces;
with Interfaces.C;
with Interfaces.C.Strings;

--
-- Interface to Agar's error handling routines. Notably, Set_Error sets the
-- error message and Get_Error returns it. In threaded builds, the error
-- message string is thread-specific (accessed via thread-local storage).
--

package Agar.Error is

  function Get_Error return String;

  procedure Set_Error (Message : in String);

  procedure Fatal_Error (Message : in String);
  
  type    Error_Callback_t          is access procedure (Message : in String);
  subtype Error_Callback_Not_Null_t is not null Error_Callback_t;

  procedure Set_Fatal_Callback
    (Callback : Error_Callback_Not_Null_t);

  private
  
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;

  type Callback_t is not null access procedure (Message : in CS.chars_ptr)
    with Convention => C;

  procedure AG_SetErrorS
    (Message : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_SetErrorS";

  procedure AG_FatalError
    (Message : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_FatalError";

  function AG_GetError return CS.chars_ptr
    with Import, Convention => C, Link_Name => "AG_GetError";

  procedure AG_SetFatalCallback (Callback : Callback_t)
    with Import, Convention => C, Link_Name => "AG_SetFatalCallback";

end Agar.Error;
