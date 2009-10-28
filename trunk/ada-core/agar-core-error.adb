with Agar.Core.Thin;
with C_String;
with Interfaces.C;

package body Agar.Core.Error is
  package C renames Interfaces.C;

  procedure Set_Error (Message : in String) is
    Ch_Message : aliased C.char_array := C.To_C (Message);
    Ch_Format  : aliased C.char_array := C.To_C ("%s");
  begin
    Thin.Error.Set_Error
      (Format => C_String.To_C_String (Ch_Format'Unchecked_Access),
       Data   => C_String.To_C_String (Ch_Message'Unchecked_Access));
  end Set_Error;

  procedure Fatal_Error (Message : in String) is
    Ch_Message : aliased C.char_array := C.To_C (Message);
    Ch_Format  : aliased C.char_array := C.To_C ("%s");
  begin
    Thin.Error.Fatal_Error
      (Format => C_String.To_C_String (Ch_Format'Unchecked_Access),
       Data   => C_String.To_C_String (Ch_Message'Unchecked_Access));
  end Fatal_Error;

  --
  -- Proxy procedure to call error callback from C code.
  --

  Error_Callback : Error_Callback_t := null;

  procedure Caller (Message : C_String.String_Not_Null_Ptr_t);
  pragma Convention (C, Caller);

  procedure Caller (Message : in C_String.String_Not_Null_Ptr_t) is
  begin
    if Error_Callback /= null then
      Error_Callback.all (C_String.To_String (Message));
    end if;
  end Caller;

  procedure Set_Fatal_Callback
    (Callback : Error_Callback_Not_Null_t) is
  begin
    Error_Callback := Callback;
    Thin.Error.Set_Fatal_Callback (Caller'Access);
  end Set_Fatal_Callback;

end Agar.Core.Error;
