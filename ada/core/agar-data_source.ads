------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                     A G A R . D A T A _ S O U R C E                      --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with System;
with Interfaces; use Interfaces;
with Interfaces.C;
with Interfaces.C.Strings; use Interfaces.C.Strings;
with Agar.Error;
with Agar.Types; use Agar.Types;

--
-- Data_Source provides basic I/O routines for serializing data to file,
-- memory or network streams.
--

package Agar.Data_Source is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;
  package ERR renames Agar.Error;
  
  LOAD_STRING_MAX : constant Natural := $AG_LOAD_STRING_MAX;

  type Data_Source is array (1 .. $SIZEOF_AG_DATASOURCE) of
    aliased Interfaces.Unsigned_8 with Convention => C;
  for Data_Source'Size use $SIZEOF_AG_DATASOURCE * System.Storage_Unit;

  type Data_Source_Access             is access all Data_Source with Convention => C;
  subtype Data_Source_Not_Null_Access is not null Data_Source_Access;

  type IO_Status is (Success, EOF, Error, Unavailable) with Convention => C;
  for IO_Status use
    (Success     => 0,
     EOF         => 1,
     Error       => 2,
     Unavailable => 3);
  for IO_Status'Size use C.unsigned'Size;

  type Seek_Mode is (Seek_Set, Seek_Current, Seek_End) with Convention => C;
  for Seek_Mode use
    (Seek_Set     => 0,
     Seek_Current => 1,
     Seek_End     => 2);
  for Seek_Mode'Size use C.unsigned'Size;

  type Byte_Order_t is (Big_Endian, Little_Endian) with Convention => C;
  for Byte_Order_t use
    (Big_Endian    => 0,
     Little_Endian => 1);
  for Byte_Order_t'Size use C.unsigned'Size;
  
  ----------------
  -- Base Types --
  ----------------
  type Signed_8 is range -127 .. 127 with Convention => C;
  for Signed_8'Size use 8;
  type Signed_16 is range -(2 **15) .. +(2 **15 - 1) with Convention => C;
  for Signed_16'Size use 16;
  type Signed_32 is range -(2 **31) .. +(2 **31 - 1) with Convention => C;
  for Signed_32'Size use 32;
  subtype Unsigned_8  is Interfaces.Unsigned_8;
  subtype Unsigned_16 is Interfaces.Unsigned_16;
  subtype Unsigned_32 is Interfaces.Unsigned_32;
#if HAVE_64BIT
  type Signed_64 is range -(2 **63) .. +(2 **63 - 1) with Convention => C;
  for Signed_64'Size use 64;
  subtype Unsigned_64 is Interfaces.Unsigned_64;
#end if;
#if HAVE_FLOAT
  subtype Float       is Interfaces.C.C_float;
  subtype Double      is Interfaces.C.double;
#end if;
  
  --
  -- Create a new Data_Source by opening a file. Modes include "r" (reading),
  -- "w" (writing) and "a" (writing at end of file).
  --
  procedure Open_File
    (Path   : in     String;
     Mode   : in     String;
     Source :    out Data_Source_Access);

  --
  -- Create a Data_Source to access Size bytes of data at address Core.
  --
  function Open_Core
    (Core : in System.Address;
     Size : in AG_Size) return Data_Source_Access
    with Import, Convention => C, Link_Name => "AG_OpenCore";

  --
  -- Create a Data_Source to access Size bytes of data (read-only) at Core.
  --
  function Open_Constant_Core
    (Core : in System.Address;
     Size : in AG_Size) return Data_Source_Access
    with Import, Convention => C, Link_Name => "AG_OpenConstCore";

  --
  -- Create a Data_Source with a dynamically-allocated buffer which will be
  -- made to grow implicitely by Write operations.
  --
  function Open_Auto_Core return Data_Source_Access
    with Import, Convention => C, Link_Name => "AG_OpenAutoCore";
  
  --
  -- Close a Data_Source. Close_Data_Source closes any type of Data_Source.
  -- The Close_* procedures are only provided for symmetry.
  --
  procedure Close_Data_Source (Source : in Data_Source_Access)
    with Import, Convention => C, Link_Name => "AG_CloseDataSource";
  procedure Close_File (Source : in Data_Source_Access)
    with Import, Convention => C, Link_Name => "AG_CloseFile";
  procedure Close_Core (Source : in Data_Source_Access)
    with Import, Convention => C, Link_Name => "AG_CloseCore";
  procedure Close_Constant_Core (Source : in Data_Source_Access)
    with Import, Convention => C, Link_Name => "AG_CloseConstCore";
  procedure Close_Auto_Core (Source : in Data_Source_Access)
    with Import, Convention => C, Link_Name => "AG_CloseAutoCore";

  --
  -- Return the current position in the Data_Source.
  --
  function Tell (Source : in Data_Source_Access) return AG_Size
    with Import, Convention => C, Link_Name => "AG_Tell";

  --
  -- Seek to specified position in the Data_Source. Mode can be Seek_Set
  -- (relative to 0), Seek_Current (relative to current position) and
  -- Seek_End (relative to end of data).
  --
  function Seek
    (Source : in Data_Source_Access;
     Offset : in AG_Offset;
     Mode   : in Seek_Mode) return C.int
    with Import, Convention => C, Link_Name => "AG_Seek";

  --
  -- Acquire/release the mutex protecting access to a Data_Source.
  --
  procedure Lock (Source : in Data_Source_Access)
    with Import, Convention => C, Link_Name => "AG_LockDataSource";
  procedure Unlock (Source : in Data_Source_Access)
    with Import, Convention => C, Link_Name => "AG_UnlockDataSource";

  --
  -- Select the preferred byte order for serialized integers.
  -- Big endian is the default.
  --
  procedure Set_Byte_Order
    (Source     : in Data_Source_Access;
     Byte_Order : in Byte_Order_t)
    with Import, Convention => C, Link_Name => "AG_SetByteOrder";

  --
  -- Embed serialization markers to enforce low-level type safety checks.
  -- A type signature will precede every data item. Note: Serialized data with
  -- debug is not compatible with serialized data produced without debug.
  --
  procedure Set_Source_Debug
    (Source : in Data_Source_Access;
     Enable : in C.int)
    with Import, Convention => C, Link_Name => "AG_SetSourceDebug";

  --
  -- Initialization/finalization routines. Implied by Open_*.
  --
  procedure Init (Source : in Data_Source_Access)
    with Import, Convention => C, Link_Name => "AG_DataSourceInit";
  procedure Destroy (Source : in Data_Source_Access)
    with Import, Convention => C, Link_Name => "AG_DataSourceDestroy";

  -- XXX TODO
  -- type Error_Func_Access is not null access procedure (Event : Event_Access);
  -- procedure Set_Error_Func
  --  (Source : in Data_Source_Access;
  --   Func   : in Event_Func_Access);
  -- pragma Import (C, Destroy, "AG_DataSourceSetErrorFn");

  -----------------
  -- Integer I/O --
  -----------------

  function Read_Unsigned_8
    (Source : in Data_Source_Access) return Interfaces.Unsigned_8
    with Import, Convention => C, Link_Name => "ag_read_uint8";
  function Read_Signed_8 (Source : in Data_Source_Access) return Signed_8
    with Import, Convention => C, Link_Name => "ag_read_sint8";

  procedure Write_Unsigned_8
    (Source : in Data_Source_Access;
     Value  : in Interfaces.Unsigned_8)
    with Import, Convention => C, Link_Name => "ag_write_uint8";
  procedure Write_Signed_8
    (Source : in Data_Source_Access;
     Value  : in Signed_8)
    with Import, Convention => C, Link_Name => "ag_write_sint8";
  procedure Write_Unsigned_8_At
    (Source : in Data_Source_Access;
     Value  : in Interfaces.Unsigned_8;
     Offset : in AG_Offset)
    with Import, Convention => C, Link_Name => "ag_write_uint8_at";
  procedure Write_Signed_8_At
    (Source : in Data_Source_Access;
     Value  : in Signed_8;
     Offset : in AG_Offset)
    with Import, Convention => C, Link_Name => "ag_write_sint8_at";

  function Read_Unsigned_16
    (Source : in Data_Source_Access) return Interfaces.Unsigned_16
    with Import, Convention => C, Link_Name => "ag_read_uint16";
  function Read_Signed_16
    (Source : in Data_Source_Access) return Signed_16
    with Import, Convention => C, Link_Name => "ag_read_sint16";
  procedure Write_Unsigned_16
    (Source : in Data_Source_Access;
     Value  : in Interfaces.Unsigned_16)
    with Import, Convention => C, Link_Name => "ag_write_uint16";
  procedure Write_Signed_16
    (Source : in Data_Source_Access;
     Value  : in Signed_16)
    with Import, Convention => C, Link_Name => "ag_write_sint16";
  procedure Write_Unsigned_16_At
    (Source : in Data_Source_Access;
     Value  : in Interfaces.Unsigned_16;
     Offset : in AG_Offset)
    with Import, Convention => C, Link_Name => "ag_write_uint16_at";
  procedure Write_Signed_16_At
    (Source : in Data_Source_Access;
     Value  : in Signed_16;
     Offset : in AG_Offset)
    with Import, Convention => C, Link_Name => "ag_write_sint16_at";

  function Read_Unsigned_32
    (Source : in Data_Source_Access) return Interfaces.Unsigned_32
    with Import, Convention => C, Link_Name => "ag_read_uint32";
  function Read_Signed_32
    (Source : in Data_Source_Access) return Signed_32
    with Import, Convention => C, Link_Name => "ag_read_sint32";
  procedure Write_Unsigned_32
    (Source : in Data_Source_Access;
     Value  : in Interfaces.Unsigned_32)
    with Import, Convention => C, Link_Name => "ag_write_uint32";
  procedure Write_Signed_32
    (Source : in Data_Source_Access;
     Value  : in Signed_32)
    with Import, Convention => C, Link_Name => "ag_write_sint32";
  procedure Write_Unsigned_32_At
    (Source : in Data_Source_Access;
     Value  : in Interfaces.Unsigned_32;
     Offset : in AG_Offset)
    with Import, Convention => C, Link_Name => "ag_write_uint32_at";
  procedure Write_Signed_32_At
    (Source : in Data_Source_Access;
     Value  : in Signed_32;
     Offset : in AG_Offset)
    with Import, Convention => C, Link_Name => "ag_write_sint32_at";

#if HAVE_64BIT
  function Read_Unsigned_64
    (Source : in Data_Source_Access) return Interfaces.Unsigned_64
    with Import, Convention => C, Link_Name => "ag_read_uint64";
  function Read_Signed_64
    (Source : in Data_Source_Access) return Signed_64
    with Import, Convention => C, Link_Name => "ag_read_sint64";
  procedure Write_Unsigned_64
    (Source : in Data_Source_Access;
     Value  : in Interfaces.Unsigned_64)
    with Import, Convention => C, Link_Name => "ag_write_uint64";
  procedure Write_Signed_64
    (Source : in Data_Source_Access;
     Value  : in Signed_64)
    with Import, Convention => C, Link_Name => "ag_write_sint64";
  procedure Write_Unsigned_64_At
    (Source : in Data_Source_Access;
     Value  : in Interfaces.Unsigned_64;
     Offset : in AG_Offset)
    with Import, Convention => C, Link_Name => "ag_write_uint64_at";
  procedure Write_Signed_64_At
    (Source : in Data_Source_Access;
     Value  : in Signed_64;
     Offset : in AG_Offset)
    with Import, Convention => C, Link_Name => "ag_write_sint64_at";
#end if;
 
  ------------------------
  -- Floating-point I/O --
  ------------------------
#if HAVE_FLOAT
  function Read_Float (Source : in Data_Source_Access) return Float
    with Import, Convention => C, Link_Name => "ag_read_float";

  procedure Write_Float
    (Source : in Data_Source_Access;
     Value  : in Float)
    with Import, Convention => C, Link_Name => "ag_write_float";

  procedure Write_Float_At
    (Source : in Data_Source_Access;
     Value  : in Float;
     Offset : in AG_Offset)
    with Import, Convention => C, Link_Name => "ag_write_float_at";

  function Read_Double (Source : in Data_Source_Access) return Double
    with Import, Convention => C, Link_Name => "ag_read_double";

  procedure Write_Double
    (Source : in Data_Source_Access;
     Value  : in Double)
    with Import, Convention => C, Link_Name => "ag_write_double";

  procedure Write_Double_At
    (Source : in Data_Source_Access;
     Value  : in Double;
     Offset : in AG_Offset)
    with Import, Convention => C, Link_Name => "ag_write_double_at";
#end if;
  
  ----------------
  -- String I/O --
  ----------------
  
  function Read_String
    (Source : in Data_Source_Access) return String;
  
  function Read_String
    (Source     : in Data_Source_Access;
     Max_Length : in Natural) return String;
  
  function Read_Padded_String
    (Source : in Data_Source_Access;
     Length : in Natural) return String;
  
  procedure Write_String
    (Source : in Data_Source_Access;
     Data   : in String);
  
  procedure Write_Padded_String
    (Source : in Data_Source_Access;
     Data   : in String;
     Length : in Natural);

  procedure Skip_String
    (Source : in Data_Source_Access)
    with Import, Convention => C, Link_Name => "AG_SkipString";
  
  procedure Skip_Padded_String
    (Source : in Data_Source_Access)
    with Import, Convention => C, Link_Name => "AG_SkipStringPadded";

  -----------------
  -- Generic I/O --
  -----------------

  generic

  type Element_Type             is private;
  type Element_Count_Type       is range <>;
  type Element_Array_Index_Type is (<>);
  type Element_Array_Type       is array (Element_Array_Index_Type range <>) of Element_Type;

  package IO is
    procedure Read
      (Source : in     Data_Source_not_null_Access;
       Buffer :    out Element_Array_Type;
       Read   :    out Element_Count_Type;
       Status :    out IO_Status);
    procedure Read_At_Offset
      (Source : in     Data_Source_not_null_Access;
       Offset : in     AG_Offset;
       Buffer :    out Element_Array_Type;
       Read   :    out Element_Count_Type;
       Status :    out IO_Status);
    procedure Write
      (Source : in     Data_Source_Not_Null_Access;
       Buffer : in     Element_Array_Type;
       Wrote  :    out Element_Count_Type;
       Status :    out IO_Status);
    procedure Write_At_Offset
      (Source : in     Data_Source_Not_Null_Access;
       Offset : in     AG_Offset;
       Buffer : in     Element_Array_Type;
       Wrote  :    out Element_Count_Type;
       Status :    out IO_Status);
  end IO;

  private

  function AG_OpenFile
    (Path : in CS.chars_ptr;
     Mode : in CS.chars_ptr) return Data_Source_Access
    with Import, Convention => C, Link_Name => "AG_OpenFile";
  
  function AG_Read
    (Source  : in Data_Source_Access;
     Buffer  : in System.Address;
     Size    : in AG_Size;
     Members : in AG_Size) return IO_Status
    with Import, Convention => C, Link_Name => "AG_Read";

  function AG_ReadAt
    (Source  : in Data_Source_Access;
     Buffer  : in System.Address;
     Size    : in AG_Size;
     Members : in AG_Size;
     Offset  : in AG_Offset) return IO_Status
    with Import, Convention => C, Link_Name => "AG_ReadAt";

  function AG_Write
    (Source  : in Data_Source_Access;
     Buffer  : in System.Address;
     Size    : in AG_Size;
     Members : in AG_Size) return IO_Status
    with Import, Convention => C, Link_Name => "AG_Write";

  function AG_WriteAt
    (Source  : in Data_Source_Access;
     Buffer  : in System.Address;
     Size    : in AG_Size;
     Members : in AG_Size;
     Offset  : in AG_Offset) return IO_Status
    with Import, Convention => C, Link_Name => "AG_WriteAt";
  
  function AG_ReadStringLen
    (Source : in Data_Source_Access;
     Max_Size : in AG_Size) return CS.chars_ptr
    with Import, Convention => C, Link_Name => "AG_ReadStringLen";

  function AG_CopyString
    (Buffer : in CS.chars_ptr;
     Source : in Data_Source_Access;
     Size   : in AG_Size) return AG_Size
    with Import, Convention => C, Link_Name => "AG_CopyString";
  
  function AG_CopyStringPadded
    (Buffer : in CS.chars_ptr;
     Source : in Data_Source_Access;
     Size   : in AG_Size) return AG_Size
    with Import, Convention => C, Link_Name => "AG_CopyStringPadded";

  procedure AG_WriteString
    (Source : in Data_Source_Access;
     Data   : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_WriteString";
  
  procedure AG_WriteStringPadded
    (Source : in Data_Source_Access;
     Data   : in CS.chars_ptr;
     Length : in AG_Size)
    with Import, Convention => C, Link_Name => "AG_WriteStringPadded";

end Agar.Data_Source;
