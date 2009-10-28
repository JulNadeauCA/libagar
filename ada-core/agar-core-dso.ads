with Agar.Core.Thin;

package Agar.Core.DSO is

  subtype DSO_Access_t          is Thin.DSO.DSO_Access_t;
  subtype DSO_Not_Null_Access_t is Thin.DSO.DSO_Not_Null_Access_t;

  function Load
    (Name : in String;
     Path : in String) return DSO_Access_t;

  function Unload (DSO : DSO_Not_Null_Access_t) return Boolean;

  function Lookup (Name : in String) return DSO_Access_t;

  procedure Lock renames Thin.DSO.Lock;

  procedure Unlock renames Thin.DSO.Unlock;

  generic
    type Subprogram_Access_Type is private;

  function Generic_Symbol_Lookup
    (DSO  : in DSO_Not_Null_Access_t;
     Name : in String) return Subprogram_Access_Type;

end Agar.Core.DSO;
