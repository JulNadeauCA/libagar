with Agar.Core.Thin;
with System;

package Agar.Core.Event is

  subtype Event_Access_t          is Thin.Event.Event_Access_t;
  subtype Event_Not_Null_Access_t is Thin.Event.Event_Not_Null_Access_t;

  subtype Long_Natural is Long_Integer range 0 .. Long_Integer'Last;

  procedure Init
    (Event : in Event_Not_Null_Access_t)
      renames Thin.Event.Init;

  procedure Push_Pointer
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in System.Address);

  procedure Push_String
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in String);

  procedure Push_Integer
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Integer);

  procedure Push_Natural
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Natural);

  procedure Push_Long
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Long_Integer);

  procedure Push_Long_Natural
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Long_Natural);

  procedure Push_Float
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Float);

  procedure Push_Long_Float
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Long_Float);

  procedure Pop_Argument
    (Event : in Event_Not_Null_Access_t)
      renames Thin.Event.Pop_Argument;

  generic
    type Element_Type        is private;
    type Element_Access_Type is access Element_Type;

  procedure Push_Generic_Access
    (Event : in Event_Not_Null_Access_t;
     Key   : in String;
     Value : in Element_Access_Type);

end Agar.Core.Event;
