------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                           A G A R . E V E N T                            --
--                                 B o d y                                  --
--                                                                          --
-- Copyright (c) 2018-2019, Julien Nadeau Carriere (vedge@csoft.net)        --
-- Copyright (c) 2010, coreland (mark@coreland.ath.cx)                      --
--                                                                          --
-- Permission to use, copy, modify, and/or distribute this software for any --
-- purpose with or without fee is hereby granted, provided that the above   --
-- copyright notice and this permission notice appear in all copies.        --
--                                                                          --
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES --
-- WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         --
-- MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  --
-- ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   --
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    --
-- ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  --
-- OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           --
------------------------------------------------------------------------------
package body Agar.Event is

  ----------------------------------
  -- Push a tagged Event Argument --
  ----------------------------------
  procedure Push_Address
    (Event : in Event_Not_Null_Access;
     Name  : in String;
     Value : in System.Address)
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    ag_event_push_pointer
      (Event => Event,
       Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access),
       Value => Value);
  end Push_Address;
  
  procedure Push_Access
    (Event : in Event_Not_Null_Access;
     Name  : in String;
     Value : in Element_Access_Type) is
  begin
    Push_Address
      (Event => Event,
       Name  => Name,
       Value => Value.all'Address);
  end Push_Access;
  
  procedure Push_String
    (Event : in Event_Not_Null_Access;
     Name  : in String;
     Value : in String)
  is
    Ch_Name  : aliased C.char_array := C.To_C(Name);
    Ch_Value : aliased C.char_array := C.To_C(Value);
  begin
    ag_event_push_string
      (Event => Event,
       Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access),
       Value => CS.To_Chars_Ptr(Ch_Value'Unchecked_Access));
  end Push_String;

  procedure Push_Integer
    (Event : in Event_Not_Null_Access;
     Name  : in String;
     Value : in Integer)
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    ag_event_push_int
      (Event => Event,
       Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access),
       Value => C.int(Value));
  end Push_Integer;

  procedure Push_Natural
    (Event : in Event_Not_Null_Access;
     Name  : in String;
     Value : in Natural)
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    ag_event_push_uint
      (Event => Event,
       Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access),
       Value => C.unsigned (Value));
  end Push_Natural;

  procedure Push_Long_Integer
    (Event : in Event_Not_Null_Access;
     Name  : in String;
     Value : in Long_Integer)
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    ag_event_push_long
      (Event => Event,
       Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access),
       Value => C.long (Value));
  end Push_Long_Integer;

#if HAVE_FLOAT
  procedure Push_Float
    (Event : in Event_Not_Null_Access;
     Name  : in String;
     Value : in Float)
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    ag_event_push_float
      (Event => Event,
       Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access),
       Value => C.C_float (Value));
  end Push_Float;

  procedure Push_Long_Float
    (Event : in Event_Not_Null_Access;
     Name  : in String;
     Value : in Long_Float)
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    ag_event_push_double
      (Event => Event,
       Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access),
       Value => C.double(Value));
  end Push_Long_Float;
#end if;
 
  -------------------------------------
  -- Push an untagged Event argument --
  -------------------------------------
  procedure Push_Address
    (Event : in Event_Not_Null_Access;
     Value : in System.Address)
  is begin
    ag_event_push_pointer
      (Event => Event,
       Name  => CS.Null_Ptr,
       Value => Value);
  end Push_Address;
  
  procedure Push_String
    (Event : in Event_Not_Null_Access;
     Value : in String)
  is
    Ch_Value : aliased C.char_array := C.To_C(Value);
  begin
    ag_event_push_string
      (Event => Event,
       Name  => CS.Null_Ptr,
       Value => CS.To_Chars_Ptr(Ch_Value'Unchecked_Access));
  end Push_String;

  procedure Push_Integer
    (Event : in Event_Not_Null_Access;
     Value : in Integer)
  is begin
    ag_event_push_int
      (Event => Event,
       Name  => CS.Null_Ptr,
       Value => C.int(Value));
  end Push_Integer;

  procedure Push_Natural
    (Event : in Event_Not_Null_Access;
     Value : in Natural)
  is begin
    ag_event_push_uint
      (Event => Event,
       Name  => CS.Null_Ptr,
       Value => C.unsigned(Value));
  end Push_Natural;

  procedure Push_Long_Integer
    (Event : in Event_Not_Null_Access;
     Value : in Long_Integer)
  is begin
    ag_event_push_long
      (Event => Event,
       Name  => CS.Null_Ptr,
       Value => C.long (Value));
  end Push_Long_Integer;

#if HAVE_FLOAT
  procedure Push_Float
    (Event : in Event_Not_Null_Access;
     Value : in Float)
  is begin
    ag_event_push_float
      (Event => Event,
       Name  => CS.Null_Ptr,
       Value => C.C_float (Value));
  end Push_Float;

  procedure Push_Long_Float
    (Event : in Event_Not_Null_Access;
     Value : in Long_Float)
  is begin
    ag_event_push_double
      (Event => Event,
       Name  => CS.Null_Ptr,
       Value => C.double(Value));
  end Push_Long_Float;
#end if;
 
  ------------------------------------
  -- Pop an untagged Event Argument --
  ------------------------------------
  
  function Pop_Address
    (Event : in Event_Not_Null_Access) return System.Address
  is begin
    return ag_event_pop_pointer (Event => Event);
  end Pop_Address;
 
  ----------------------------
  -- Extract Event Argument --
  ----------------------------
  
  function Get_Address
    (Event : in Event_Not_Null_Access;
     Index : in Natural) return System.Address
  is begin
    return ag_event_get_ptr
      (Event => Event,
       Index => C.unsigned(Index));
  end Get_Address;

  function Get_Address
    (Event : in Event_Not_Null_Access;
     Name  : in String) return System.Address
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    return ag_event_get_ptr_named
      (Event => Event,
       Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access));
  end Get_Address;
  
  function Get_String
    (Event : in Event_Not_Null_Access;
     Index : in Natural) return String
  is
    Result : CS.chars_ptr;
  begin
    Result := ag_event_get_string
      (Event => Event,
       Index => C.unsigned(Index));
    return C.To_Ada (CS.Value(Result));
  end Get_String;
  
  function Get_String
    (Event : in Event_Not_Null_Access;
     Name  : in String) return String
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
    Result  : CS.chars_ptr;
  begin
    Result := ag_event_get_string_named
      (Event => Event,
       Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access));
    return C.To_Ada(CS.Value(Result));
  end Get_String;
  
  function Get_Integer
    (Event : in Event_Not_Null_Access;
     Index : in Natural) return Integer
  is begin
    return Integer
      (ag_event_get_int
        (Event => Event,
         Index => C.unsigned(Index)));
  end Get_Integer;
  
  function Get_Integer
    (Event : in Event_Not_Null_Access;
     Name  : in String) return Integer
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    return Integer
      (ag_event_get_int_named
        (Event => Event,
         Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access)));
  end Get_Integer;
  
  function Get_Natural
    (Event : in Event_Not_Null_Access;
     Index : in Natural) return Natural
  is begin
    return Natural
      (ag_event_get_uint
        (Event => Event,
         Index => C.unsigned(Index)));
  end Get_Natural;
  
  function Get_Natural
    (Event : in Event_Not_Null_Access;
     Name  : in String) return Natural
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    return Natural
      (ag_event_get_uint_named
        (Event => Event,
         Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access)));
  end Get_Natural;
  
  function Get_Long_Integer
    (Event : in Event_Not_Null_Access;
     Index : in Natural) return Long_Integer
  is begin
    return Long_Integer
      (ag_event_get_long
        (Event => Event,
         Index => C.unsigned(Index)));
  end Get_Long_Integer;

  function Get_Long_Integer
    (Event : in Event_Not_Null_Access;
     Name  : in String) return Long_Integer
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    return Long_Integer
      (ag_event_get_long_named
        (Event => Event,
         Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access)));
  end Get_Long_Integer;

#if HAVE_FLOAT 
  function Get_Float
    (Event : in Event_Not_Null_Access;
     Index : in Natural) return Float
  is begin
    return Float
      (ag_event_get_float
        (Event => Event,
	 Index => C.unsigned(Index)));
  end Get_Float;

  function Get_Float
    (Event : in Event_Not_Null_Access;
     Name  : in String) return Float
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    return Float
      (ag_event_get_float_named
        (Event => Event,
         Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access)));
  end Get_Float;
  
  function Get_Long_Float
    (Event : in Event_Not_Null_Access;
     Index : in Natural) return Long_Float
  is begin
    return Long_Float
      (ag_event_get_double
        (Event => Event,
	 Index => C.unsigned(Index)));
  end Get_Long_Float;

  function Get_Long_Float
    (Event : in Event_Not_Null_Access;
     Name  : in String) return Long_Float
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    return Long_Float
      (ag_event_get_double_named
        (Event => Event,
	 Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access)));
  end Get_Long_Float;
#end if;

end Agar.Event;
