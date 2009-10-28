with Agar.Core.Thin;
with Interfaces;

package Agar.Core.Data_Source is

  subtype Data_Source_Access_t          is Thin.Data_Source.Data_Source_Access_t;
  subtype Data_Source_Not_Null_Access_t is Thin.Data_Source.Data_Source_Not_Null_Access_t;

  --
  -- Open.
  --

  procedure Open_File
    (Path   : in     String;
     Mode   : in     String;
     Source :    out Data_Source_Access_t);

  --
  -- I/O
  --

  type    IO_Status_t   is new Thin.Data_Source.IO_Status_t;
  subtype Byte_Offset_t is Interfaces.Unsigned_64;

  generic
    type Element_Type             is private;
    type Element_Count_Type       is range <>;
    type Element_Array_Index_Type is (<>);
    type Element_Array_Type       is array (Element_Array_Index_Type range <>) of Element_Type;

  package IO is

    procedure Read
      (Source : in     Data_Source_Not_Null_Access_t;
       Buffer :    out Element_Array_Type;
       Read   :    out Element_Count_Type;
       Status :    out IO_Status_t);

    procedure Read_At_Offset
      (Source : in     Data_Source_Not_Null_Access_t;
       Offset : in     Byte_Offset_t;
       Buffer :    out Element_Array_Type;
       Read   :    out Element_Count_Type;
       Status :    out IO_Status_t);

    procedure Write
      (Source : in     Data_Source_Not_Null_Access_t;
       Buffer : in     Element_Array_Type;
       Wrote  :    out Element_Count_Type;
       Status :    out IO_Status_t);

    procedure Write_At_Offset
      (Source : in     Data_Source_Not_Null_Access_t;
       Offset : in     Byte_Offset_t;
       Buffer : in     Element_Array_Type;
       Wrote  :    out Element_Count_Type;
       Status :    out IO_Status_t);

  end IO;

  --
  -- Close.
  --

  procedure Close_File
    (Source : in Data_Source_Not_Null_Access_t);

end Agar.Core.Data_Source;
