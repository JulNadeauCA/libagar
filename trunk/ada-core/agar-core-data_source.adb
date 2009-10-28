with C_String;
with Interfaces.C;
with System;

package body Agar.Core.Data_Source is
  package C renames Interfaces.C;

  use type C.size_t;

  --
  -- Open.
  --

  procedure Open_File
    (Path   : in     String;
     Mode   : in     String;
     Source :    out Data_Source_Access_t)
  is
    Ch_Path : aliased C.char_array := C.To_C (Path);
    Ch_Mode : aliased C.char_array := C.To_C (Mode);
  begin
    Source := Thin.Data_Source.Open_File
      (Path => C_String.To_C_String (Ch_Path'Unchecked_Access),
       Mode => C_String.To_C_String (Ch_Mode'Unchecked_Access));
  end Open_File;

  --
  -- Close.
  --

  procedure Close_File
    (Source : in Data_Source_Not_Null_Access_t) is
  begin
    Thin.Data_Source.Close_File (Source);
  end Close_File;

  --
  -- I/O.
  --

  package body IO is

    Element_Bytes : constant C.size_t := Element_Type'Size / System.Storage_Unit;

    procedure Read
      (Source : in     Data_Source_Not_Null_Access_t;
       Buffer :    out Element_Array_Type;
       Read   :    out Element_Count_Type;
       Status :    out IO_Status_t) is
    begin
      Status := IO_Status_t (Thin.Data_Source.Read
        (Source  => Source,
         Buffer  => Buffer (Buffer'First)'Address,
         Size    => Element_Bytes,
         Members => Buffer'Length));
      if Status = Success then
        Read := Buffer'Length;
      end if;
    end Read;

    procedure Read_At_Offset
      (Source : in     Data_Source_Not_Null_Access_t;
       Offset : in     Byte_Offset_t;
       Buffer :    out Element_Array_Type;
       Read   :    out Element_Count_Type;
       Status :    out IO_Status_t) is
    begin
      Status := IO_Status_t (Thin.Data_Source.Read_At
        (Source  => Source,
         Buffer  => Buffer (Buffer'First)'Address,
         Size    => Element_Bytes,
         Members => Buffer'Length,
         Offset  => C.size_t (Offset)));
      if Status = Success then
        Read := Buffer'Length;
      end if;
    end Read_At_Offset;

    procedure Write
      (Source : in     Data_Source_Not_Null_Access_t;
       Buffer : in     Element_Array_Type;
       Wrote  :    out Element_Count_Type;
       Status :    out IO_Status_t) is
    begin
      Status := IO_Status_t (Thin.Data_Source.Write
        (Source  => Source,
         Buffer  => Buffer (Buffer'First)'Address,
         Size    => Element_Bytes,
         Members => Buffer'Length));
      if Status = Success then
        Wrote := Buffer'Length;
      end if;
    end Write;

    procedure Write_At_Offset
      (Source : in     Data_Source_Not_Null_Access_t;
       Offset : in     Byte_Offset_t;
       Buffer : in     Element_Array_Type;
       Wrote  :    out Element_Count_Type;
       Status :    out IO_Status_t) is
    begin
      Status := IO_Status_t (Thin.Data_Source.Write_At
        (Source  => Source,
         Buffer  => Buffer (Buffer'First)'Address,
         Size    => Element_Bytes,
         Offset  => C.size_t (Offset),
         Members => Buffer'Length));
      if Status = Success then
        Wrote := Buffer'Length;
      else
        Wrote := 0;
      end if;
    end Write_At_Offset;

  end IO;

end Agar.Core.Data_Source;
