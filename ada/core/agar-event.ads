------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                           A G A R . E V E N T                            --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with System;
with Interfaces.C;
with Interfaces.C.Strings;

--
-- Argument passing and retrieval for event handler routines (and virtual
-- functions of Agar objects in general). Arguments are of type Agar.Variable,
-- which includes discrete and pointer types. Arguments can be tagged and
-- accessed by index or name.
--
-- NOTE: Agar enforces run-time type safety only when it is compiled with
--       the configure option --enable-type-safety.
--

package Agar.Event is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;

  ARGS_MAX : constant Natural := $AG_EVENT_ARGS_MAX;
  NAME_MAX : constant Natural := $AG_EVENT_NAME_MAX;
  
  type Event is array (1 .. $SIZEOF_AG_EVENT) of
    aliased Interfaces.Unsigned_8 with Convention => C;
  for Event'Size use $SIZEOF_AG_EVENT * System.Storage_Unit;

  type Event_Access             is access all Event with Convention => C;
  subtype Event_not_null_Access is not null Event_Access;

  --
  -- Initialize an existing Event.
  --
  procedure Init (Event : in Event_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_EventInit";

  --
  -- Push a tagged Event argument.
  --
  procedure Push_Address
    (Event : in Event_not_null_Access;
     Name  : in String;
     Value : in System.Address);
  procedure Push_String
    (Event : in Event_not_null_Access;
     Name  : in String;
     Value : in String);
  procedure Push_Integer
    (Event : in Event_not_null_Access;
     Name  : in String;
     Value : in Integer);
  procedure Push_Natural
    (Event : in Event_not_null_Access;
     Name  : in String;
     Value : in Natural);
  procedure Push_Long_Integer
    (Event : in Event_not_null_Access;
     Name  : in String;
     Value : in Long_Integer);
#if HAVE_FLOAT
  procedure Push_Float
    (Event : in Event_not_null_Access;
     Name  : in String;
     Value : in Float);
  procedure Push_Long_Float
    (Event : in Event_not_null_Access;
     Name  : in String;
     Value : in Long_Float);
#end if;
 
  --
  -- Push an untagged Event argument.
  --
  procedure Push_Address
    (Event : in Event_not_null_Access;
     Value : in System.Address);
  procedure Push_String
    (Event : in Event_not_null_Access;
     Value : in String);
  procedure Push_Integer
    (Event : in Event_not_null_Access;
     Value : in Integer);
  procedure Push_Natural
    (Event : in Event_not_null_Access;
     Value : in Natural);
  procedure Push_Long_Integer
    (Event : in Event_not_null_Access;
     Value : in Long_Integer);
#if HAVE_FLOAT
  procedure Push_Float
    (Event : in Event_not_null_Access;
     Value : in Float);
  procedure Push_Long_Float
    (Event : in Event_not_null_Access;
     Value : in Long_Float);
#end if; 
 
  --
  -- Pop the last argument off an Event argument stack
  --
  function Pop_Address
    (Event : in Event_not_null_Access) return System.Address;

  --
  -- Extract an argument by index.
  --
  function Get_Address
    (Event : in Event_not_null_Access;
     Index : in Natural) return System.Address;
  function Get_String
    (Event : in Event_not_null_Access;
     Index : in Natural) return String;
  function Get_Integer
    (Event : in Event_not_null_Access;
     Index : in Natural) return Integer;
  function Get_Natural
    (Event : in Event_not_null_Access;
     Index : in Natural) return Natural;
  function Get_Long_Integer
    (Event : in Event_not_null_Access;
     Index : in Natural) return Long_Integer;
#if HAVE_FLOAT
  function Get_Float
    (Event : in Event_not_null_Access;
     Index : in Natural) return Float;
  function Get_Long_Float
    (Event : in Event_not_null_Access;
     Index : in Natural) return Long_Float;
#end if; 
 
  --
  -- Extract an argument by tag.
  --
  function Get_Address
    (Event : in Event_not_null_Access;
     Name  : in String) return System.Address;
  function Get_String
    (Event : in Event_not_null_Access;
     Name  : in String) return String;
  function Get_Integer
    (Event : in Event_not_null_Access;
     Name  : in String) return Integer;
  function Get_Natural
    (Event : in Event_not_null_Access;
     Name  : in String) return Natural;
  function Get_Long_Integer
    (Event : in Event_not_null_Access;
     Name  : in String) return Long_Integer;
#if HAVE_FLOAT
  function Get_Float
    (Event : in Event_not_null_Access;
     Name  : in String) return Float;
  function Get_Long_Float
    (Event : in Event_not_null_Access;
     Name  : in String) return Long_Float;
#end if;

  generic
  type Element_Type        is private;
  type Element_Access_Type is access Element_Type;
  procedure Push_Access
    (Event : in Event_not_null_Access;
     Name  : in String;
     Value : in Element_Access_Type);
  
  private

  --
  -- Push argument
  --

  procedure ag_event_push_pointer
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr;
     Value : in System.Address)
    with Import, Convention => C;
  
  procedure ag_event_push_string
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr;
     Value : in CS.chars_ptr)
    with Import, Convention => C;
  
  procedure ag_event_push_int
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr;
     Value : in C.int)
    with Import, Convention => C;

  procedure ag_event_push_uint
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr;
     Value : in C.unsigned)
    with Import, Convention => C;

  procedure ag_event_push_long
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr;
     Value : in C.long)
    with Import, Convention => C;

#if HAVE_FLOAT
  procedure ag_event_push_float
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr;
     Value : in C.C_float)
    with Import, Convention => C;

  procedure ag_event_push_double
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr;
     Value : in C.double)
    with Import, Convention => C;
#end if;

  function ag_event_pop_pointer
    (Event : in Event_not_null_Access) return System.Address
    with Import, Convention => C;
  
  --
  -- Extract argument by index
  --

  function ag_event_get_ptr
    (Event : in Event_not_null_Access;
     Index : in C.unsigned) return System.Address
    with Import, Convention => C;

  function ag_event_get_const_ptr
    (Event : in Event_not_null_Access;
     Index : in C.unsigned) return System.Address
    with Import, Convention => C;
  
  function ag_event_get_string
    (Event : in Event_not_null_Access;
     Index : in C.unsigned) return CS.chars_ptr
    with Import, Convention => C;
  
  function ag_event_get_int
    (Event : in Event_not_null_Access;
     Index : in C.unsigned) return C.int
    with Import, Convention => C;
  
  function ag_event_get_uint
    (Event : in Event_not_null_Access;
     Index : in C.unsigned) return C.unsigned
    with Import, Convention => C;
  
  function ag_event_get_long
    (Event : in Event_not_null_Access;
     Index : in C.unsigned) return C.long
    with Import, Convention => C;
 
#if HAVE_FLOAT 
  function ag_event_get_float
    (Event : in Event_not_null_Access;
     Index : in C.unsigned) return C.C_float
    with Import, Convention => C;
  
  function ag_event_get_double
    (Event : in Event_not_null_Access;
     Index : in C.unsigned) return C.double
    with Import, Convention => C;
#end if;

  --
  -- Extract argument by name
  --

  function ag_event_get_ptr_named
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr) return System.Address
    with Import, Convention => C;
  
  function ag_event_get_string_named
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr) return CS.chars_ptr
    with Import, Convention => C;
  
  function ag_event_get_int_named
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr) return C.int
    with Import, Convention => C;
  
  function ag_event_get_uint_named
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr) return C.unsigned
    with Import, Convention => C;

  function ag_event_get_long_named
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr) return C.long
    with Import, Convention => C;
 
#if HAVE_FLOAT 
  function ag_event_get_float_named
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr) return C.C_float
    with Import, Convention => C;
  
  function ag_event_get_double_named
    (Event : in Event_not_null_Access;
     Name  : in CS.chars_ptr) return C.double
    with Import, Convention => C;
#end if;

end Agar.Event;
