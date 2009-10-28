with C_String;
with Interfaces.C;

package body Agar.Core.Event is
  package C renames Interfaces.C;

  procedure Push_Pointer
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in System.Address)
  is
    Ch_Key : aliased C.char_array := C.To_C (Key);
  begin
    Thin.Event.Push_Pointer
      (Event => Event,
       Key   => C_String.To_C_String (Ch_Key'Unchecked_Access),
       Value => Value);
  end Push_Pointer;

  procedure Push_String
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in String)
  is
    Ch_Key   : aliased C.char_array := C.To_C (Key);
    Ch_Value : aliased C.char_array := C.To_C (Value);
  begin
    Thin.Event.Push_String
      (Event => Event,
       Key   => C_String.To_C_String (Ch_Key'Unchecked_Access),
       Value => C_String.To_C_String (Ch_Value'Unchecked_Access));
  end Push_String;

  procedure Push_Integer
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Integer)
  is
    Ch_Key : aliased C.char_array := C.To_C (Key);
  begin
    Thin.Event.Push_Integer
      (Event => Event,
       Key   => C_String.To_C_String (Ch_Key'Unchecked_Access),
       Value => C.int (Value));
  end Push_Integer;

  procedure Push_Natural
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Natural)
  is
    Ch_Key : aliased C.char_array := C.To_C (Key);
  begin
    Thin.Event.Push_Unsigned_Integer
      (Event => Event,
       Key   => C_String.To_C_String (Ch_Key'Unchecked_Access),
       Value => C.unsigned (Value));
  end Push_Natural;

  procedure Push_Long
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Long_Integer)
  is
    Ch_Key : aliased C.char_array := C.To_C (Key);
  begin
    Thin.Event.Push_Long
      (Event => Event,
       Key   => C_String.To_C_String (Ch_Key'Unchecked_Access),
       Value => C.long (Value));
  end Push_Long;

  procedure Push_Long_Natural
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Long_Natural)
  is
    Ch_Key : aliased C.char_array := C.To_C (Key);
  begin
    Thin.Event.Push_Unsigned_Long
      (Event => Event,
       Key   => C_String.To_C_String (Ch_Key'Unchecked_Access),
       Value => C.unsigned_long (Value));
  end Push_Long_Natural;

  procedure Push_Float
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Float)
  is
    Ch_Key : aliased C.char_array := C.To_C (Key);
  begin
    Thin.Event.Push_Float
      (Event => Event,
       Key   => C_String.To_C_String (Ch_Key'Unchecked_Access),
       Value => C.C_float (Value));
  end Push_Float;

  procedure Push_Long_Float
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Long_Float)
  is
    Ch_Key : aliased C.char_array := C.To_C (Key);
  begin
    Thin.Event.Push_Double
      (Event => Event,
       Key   => C_String.To_C_String (Ch_Key'Unchecked_Access),
       Value => C.double (Value));
  end Push_Long_Float;

  procedure Push_Generic_Access
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Element_Access_Type) is
  begin
    Push_Pointer
      (Event => Event,
       Key   => Key,
       Value => Value.all'Address);
  end Push_Generic_Access;

end Agar.Core.Event;
