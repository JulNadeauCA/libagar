------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                     A G A R . D A T A _ S O U R C E                      --
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
package body Agar.Data_Source is
  procedure Open_File
    (Path   : in     String;
     Mode   : in     String;
     Source :    out Data_Source_Access)
  is
    Ch_Path : aliased C.char_array := C.To_C (Path);
    Ch_Mode : aliased C.char_array := C.To_C (Mode);
  begin
    Source := AG_OpenFile
      (Path => To_Chars_Ptr(Ch_Path'Unchecked_Access),
       Mode => To_Chars_Ptr(Ch_Mode'Unchecked_Access));
  end;

  package body IO is

    Element_Bytes : constant AG_Size := Element_Type'Size / System.Storage_Unit;

    procedure Read
      (Source : in     Data_Source_Not_Null_Access;
       Buffer :    out Element_Array_Type;
       Read   :    out Element_Count_Type;
       Status :    out IO_Status) is
    begin
      Status := AG_Read
        (Source  => Source,
         Buffer  => Buffer (Buffer'First)'Address,
         Size    => Element_Bytes,
         Members => Buffer'Length);
      if Status = Success then
        Read := Buffer'Length;
      end if;
    end Read;

    procedure Read_At_Offset
      (Source : in     Data_Source_Not_Null_Access;
       Offset : in     AG_Offset;
       Buffer :    out Element_Array_Type;
       Read   :    out Element_Count_Type;
       Status :    out IO_Status) is
    begin
      Status := AG_ReadAt
        (Source  => Source,
         Buffer  => Buffer (Buffer'First)'Address,
         Size    => Element_Bytes,
         Members => Buffer'Length,
         Offset  => Offset);
      if Status = Success then
        Read := Buffer'Length;
      end if;
    end Read_At_Offset;

    procedure Write
      (Source : in     Data_Source_Not_Null_Access;
       Buffer : in     Element_Array_Type;
       Wrote  :    out Element_Count_Type;
       Status :    out IO_Status) is
    begin
      Status := AG_Write
        (Source  => Source,
         Buffer  => Buffer (Buffer'First)'Address,
         Size    => Element_Bytes,
         Members => Buffer'Length);
      if Status = Success then
        Wrote := Buffer'Length;
      end if;
    end;

    procedure Write_At_Offset
      (Source : in     Data_Source_Not_Null_Access;
       Offset : in     AG_Offset;
       Buffer : in     Element_Array_Type;
       Wrote  :    out Element_Count_Type;
       Status :    out IO_Status) is
    begin
      Status := AG_WriteAt
        (Source  => Source,
         Buffer  => Buffer (Buffer'First)'Address,
         Size    => Element_Bytes,
         Offset  => Offset,
         Members => Buffer'Length);
      if Status = Success then
        Wrote := Buffer'Length;
      else
        Wrote := 0;
      end if;
    end;

  end IO;
  
  function Read_String (Source : in Data_Source_Access) return String
  is
    Result : chars_ptr;
  begin
    Result := AG_ReadStringLen(Source, AG_Size(LOAD_STRING_MAX));
    if Result = Null_Ptr then
      raise Program_Error with ERR.Get_Error;
    end if;
    -- XXX FIXME leak
    return C.To_Ada(Value(Result));
  end;
  
  function Read_String
    (Source     : in Data_Source_Access;
     Max_Length : in Natural) return String
  is
    Result : chars_ptr;
  begin
    Result := AG_ReadStringLen(Source, AG_Size(Max_Length));
    if Result = Null_Ptr then
      raise Program_Error with ERR.Get_Error;
    end if;
    -- XXX FIXME leak
    return C.To_Ada(Value(Result));
  end;
  
  function Read_Padded_String
    (Source : in Data_Source_Access;
     Length : in Natural) return String
  is
    Ch_Name : aliased C.char_array := (1 .. C.size_t(Length) => C.nul);
    Result  : AG_Size;
  begin
    Result := AG_CopyStringPadded
      (Buffer => To_Chars_Ptr(Ch_Name'Unchecked_Access),
       Source => Source,
       Size   => Ch_Name'Length);
    if Integer(Result) = 0 then
      raise Program_Error with ERR.Get_Error;
    end if;
    return C.To_Ada(Ch_Name);
  end;

  procedure Write_String
    (Source : in Data_Source_Access;
     Data   : in String)
  is
    Ch_Data : aliased C.char_array := C.To_C(Data);
  begin
    AG_WriteString
      (Source => Source,
       Data   => To_Chars_Ptr(Ch_Data'Unchecked_Access));
  end;
  
  procedure Write_Padded_String
    (Source : in Data_Source_Access;
     Data   : in String;
     Length : in Natural)
  is
    Ch_Data : aliased C.char_array := C.To_C(Data);
  begin
    AG_WriteStringPadded
      (Source => Source,
       Data   => To_Chars_Ptr(Ch_Data'Unchecked_Access),
       Length => AG_Size(Length));
  end;

end Agar.Data_Source;
