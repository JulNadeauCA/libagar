with Ada.Unchecked_Conversion;
with C_String;
with Interfaces.C;
with System;

package body Agar.Core.DSO is
  package C renames Interfaces.C;

  use type C.int;

  function Load
    (Name : in String;
     Path : in String) return DSO_Access_t
  is
    Ch_Name : aliased C.char_array := C.To_C (Name);
    Ch_Path : aliased C.char_array := C.To_C (Path);
  begin
    return Thin.DSO.Load
      (Name  => C_String.To_C_String (Ch_Name'Unchecked_Access),
       Path  => C_String.To_C_String (Ch_Path'Unchecked_Access),
       Flags => 0);
  end Load;

  function Unload (DSO : DSO_Not_Null_Access_t) return Boolean is
  begin
    return 1 = Thin.DSO.Unload (DSO);
  end Unload;

  function Lookup (Name : in String) return DSO_Access_t is
    Ch_Name : aliased C.char_array := C.To_C (Name);
  begin
    return Thin.DSO.Lookup (C_String.To_C_String (Ch_Name'Unchecked_Access));
  end Lookup;

  function Generic_Symbol_Lookup
    (DSO  : in DSO_Not_Null_Access_t;
     Name : in String) return Subprogram_Access_Type
  is
    Ch_Name : aliased C.char_array := C.To_C (Name);
    Result  : aliased System.Address;

    -- XXX: Risky...
    function Convert is new Ada.Unchecked_Conversion
      (Source => System.Address,
       Target => Subprogram_Access_Type);
  begin
    if 1 = Thin.DSO.Symbol
      (DSO   => DSO,
       Name  => C_String.To_C_String (Ch_Name'Unchecked_Access),
       Value => Result'Unchecked_Access) then
      return Convert (Result);
    else
      return Convert (System.Null_Address);
    end if;
  end Generic_Symbol_Lookup;

end Agar.Core.DSO;
