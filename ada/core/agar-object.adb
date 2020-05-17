------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                          A G A R . O B J E C T                           --
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
with Agar.Error;

package body Agar.Object is
  use Interfaces.C;
  use Agar.Event;

  function New_Object
    (Parent : in Object_Access;
     Name   : in String;
     Class  : in Class_Not_Null_Access) return Object_Access
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    return AG_ObjectNew
      (Parent => Parent.all'Address,
       Name   => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access),
       Class  => Class);
  end;
  
  function New_Object
    (Parent : in Object_Access;
     Class  : in Class_Not_Null_Access) return Object_Access is
  begin
    return AG_ObjectNew
      (Parent => Parent.all'Address,
       Name   => CS.Null_Ptr,
       Class  => Class);
  end;
  
  function New_Object
    (Class : in Class_Not_Null_Access) return Object_Access is
  begin
    return AG_ObjectNew
      (Parent => System.Null_Address,
       Name   => CS.Null_Ptr,
       Class  => Class);
  end;

  procedure Init_Object
    (Object         : in Object_Not_Null_Access;
     Class          : in Class_Not_Null_Access;
     Static         : in Boolean := False;
     Name_On_Attach : in Boolean := False)
  is
    C_Flags : C.unsigned := 0;
  begin
    AG_ObjectInit (Object, Class);
    C_Flags := Object.Flags;
    if Static then
      C_Flags := C_Flags or OBJECT_STATIC;
    end if;
    if Name_On_Attach then
      C_Flags := C_Flags or OBJECT_NAME_ON_ATTACH;
    end if;
    Object.Flags := C_Flags;
  end;
  
  procedure Attach
    (Parent : in Object_Access;
     Child  : in Object_not_null_Access)
  is
  begin
    AG_ObjectAttach
      (Parent => Parent,
       Child  => Child);
  end;

  function Find
    (Root : in Object_Not_Null_Access;
     Path : in String) return Object_Access
  is
    Ch_Path : aliased C.char_array := C.To_C(Path);
  begin
    return AG_ObjectFindS
      (Root => Root,
       Path => CS.To_Chars_Ptr(Ch_Path'Unchecked_Access));
  end;

  function Find_Parent
    (Object : in Object_Not_Null_Access;
     Name   : in String;
     Class  : in String) return Object_Access
  is
    Ch_Name  : aliased C.char_array := C.To_C(Name);
    Ch_Class : aliased C.char_array := C.To_C(Class);
  begin
    return AG_ObjectFindParent
      (Object => Object,
       Name   => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access),
       Class  => CS.To_Chars_Ptr(Ch_Class'Unchecked_Access));
  end;

  function Find_Child
    (Parent : in Object_Not_Null_Access;
     Name   : in String) return Object_Access
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    return AG_ObjectFindChild
      (Parent => Parent,
       Name   => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access));
  end;

  function Get_Name (Object : in Object_Not_Null_Access) return String
  is
    Ch_Name : aliased C.char_array (0 .. C.size_t(HIERARCHY_MAX));
    Result  : C.int;
  begin
    Result := AG_ObjectCopyName
      (Object => Object,
       Buffer => Ch_Name'Address,
       Size   => Ch_Name'Length);
    if Integer(Result) /= 0 then
      raise Program_Error with Agar.Error.Get_Error;
    end if;
    return C.To_Ada(Ch_Name);
  end;

  procedure Set_Name
    (Object : in Object_Not_Null_Access;
     Name   : in String)
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    AG_ObjectSetNameS
      (Object => Object,
       Name   => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access));
  end;

  function Generate_Name
    (Object : in Object_Not_Null_Access;
     Class  : in Class_Not_Null_Access) return String
  is
    Ch_Name : aliased C.char_array (0 .. C.size_t(NAME_MAX));
  begin
    AG_ObjectGenName
      (Object => Object,
       Class  => Class,
       Buffer => Ch_Name'Address,
       Size   => Ch_Name'Length);
    return C.To_Ada(Ch_Name);
  end;
  
  function Generate_Name
    (Object : in Object_Not_Null_Access;
     Prefix : in String) return String
  is
    Ch_Name   : aliased C.char_array (0 .. C.size_t(NAME_MAX));
    Ch_Prefix : aliased C.char_array := C.To_C(Prefix);
  begin
    AG_ObjectGenNamePfx
      (Object => Object,
       Prefix => CS.To_Chars_Ptr(Ch_Prefix'Unchecked_Access),
       Buffer => Ch_Name'Address,
       Size   => Ch_Name'Length);
    return C.To_Ada(Ch_Name);
  end;

  procedure Register_Namespace
    (Name   : in String;
     Prefix : in String;
     URL    : in String)
  is
    Ch_Name   : aliased C.char_array := C.To_C(Name);
    Ch_Prefix : aliased C.char_array := C.To_C(Prefix);
    Ch_URL    : aliased C.char_array := C.To_C(URL);
  begin
    AG_RegisterNamespace
      (Name   => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access),
       Prefix => CS.To_Chars_Ptr(Ch_Prefix'Unchecked_Access),
       URL    => CS.To_Chars_Ptr(Ch_URL'Unchecked_Access));
  end;

  procedure Unregister_Namespace
    (Name : in String)
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    AG_UnregisterNamespace(CS.To_Chars_Ptr(Ch_Name'Unchecked_Access));
  end;

  function Create_Class
    (Hierarchy    : in String;
     Object_Size  : in Natural;
     Class_Size   : in Natural;
     Major        : in Natural := 1;
     Minor        : in Natural := 0;
     Init_Func    : in Init_Func_Access := null;
     Reset_Func   : in Reset_Func_Access := null;
     Destroy_Func : in Destroy_Func_Access := null;
     Load_Func    : in Load_Func_Access := null;
     Save_Func    : in Save_Func_Access := null;
     Edit_Func    : in Edit_Func_Access := null) return Class_Not_Null_Access
  is
    Ch_Hierarchy : aliased C.char_array := C.To_C(Hierarchy);
    Class        : Class_Access;
  begin
    Class := AG_CreateClass
      (Hierarchy   => CS.To_Chars_Ptr(Ch_Hierarchy'Unchecked_Access),
       Object_Size => AG_Size(Object_Size / System.Storage_Unit),
       Class_Size  => AG_Size(Class_Size / System.Storage_Unit),
       Major       => C.unsigned(Major),
       Minor       => C.unsigned(Minor));
    if Class = null then
      raise Program_Error with Agar.Error.Get_Error;
    end if;
    if Init_Func    /= null then AG_ClassSetInit    (Class, Init_Func);    end if;
    if Reset_Func   /= null then AG_ClassSetReset   (Class, Reset_Func);   end if;
    if Destroy_Func /= null then AG_ClassSetDestroy (Class, Destroy_Func); end if;
    if Load_Func    /= null then AG_ClassSetLoad    (Class, Load_Func);    end if;
    if Save_Func    /= null then AG_ClassSetSave    (Class, Save_Func);    end if;
    if Edit_Func    /= null then AG_ClassSetEdit    (Class, Edit_Func);    end if;
    return Class;
  end Create_Class;
  
  procedure Class_Set_Init
    (Class     : in Class_Not_Null_Access;
     Init_Func : in Init_Func_Access) is
  begin
    AG_ClassSetInit(Class, Init_Func);
  end;

  function Class_Set_Init
    (Class     : in Class_Not_Null_Access;
     Init_Func : in Init_Func_Access) return Init_Func_Access is
  begin
    return AG_ClassSetInit(Class, Init_Func);
  end;
  
  procedure Class_Set_Reset
    (Class      : in Class_Not_Null_Access;
     Reset_Func : in Reset_Func_Access) is
  begin
    AG_ClassSetReset(Class, Reset_Func);
  end;
  
  function Class_Set_Reset
    (Class      : in Class_Not_Null_Access;
     Reset_Func : in Reset_Func_Access) return Reset_Func_Access is
  begin
    return AG_ClassSetReset(Class, Reset_Func);
  end;
  
  procedure Class_Set_Destroy
    (Class        : in Class_Not_Null_Access;
     Destroy_Func : in Destroy_Func_Access) is
  begin
    AG_ClassSetDestroy(Class, Destroy_Func);
  end;
  
  function Class_Set_Destroy
    (Class        : in Class_Not_Null_Access;
     Destroy_Func : in Destroy_Func_Access) return Destroy_Func_Access is
  begin
    return AG_ClassSetDestroy(Class, Destroy_Func);
  end;
  
  procedure Class_Set_Load
    (Class     : in Class_Not_Null_Access;
     Load_Func : in Load_Func_Access) is
  begin
    AG_ClassSetLoad(Class, Load_Func);
  end;
  
  function Class_Set_Load
    (Class     : in Class_Not_Null_Access;
     Load_Func : in Load_Func_Access) return Load_Func_Access is
  begin
    return AG_ClassSetLoad(Class, Load_Func);
  end;
  
  procedure Class_Set_Save
    (Class     : in Class_Not_Null_Access;
     Save_Func : in Save_Func_Access) is
  begin
    AG_ClassSetSave(Class, Save_Func);
  end;
  function Class_Set_Save
    (Class     : in Class_Not_Null_Access;
     Save_Func : in Save_Func_Access) return Save_Func_Access is
  begin
    return AG_ClassSetSave(Class, Save_Func);
  end;
  
  procedure Class_Set_Edit
    (Class     : in Class_Not_Null_Access;
     Edit_Func : in Edit_Func_Access) is
  begin
    AG_ClassSetEdit(Class, Edit_Func);
  end;
  function Class_Set_Edit
    (Class     : in Class_Not_Null_Access;
     Edit_Func : in Edit_Func_Access) return Edit_Func_Access is
  begin
    return AG_ClassSetEdit(Class, Edit_Func);
  end;
  
  function Lookup_Class
    (Class : in String) return Class_Access
  is
    Ch_Class : aliased C.char_array := C.To_C(Class);
  begin
    return AG_LookupClass
      (Class => CS.To_Chars_Ptr(Ch_Class'Unchecked_Access));
  end;

  function Load_Class
    (Class : in String) return Class_Access
  is
    Ch_Class : aliased C.char_array := C.To_C(Class);
  begin
    return AG_LoadClass
      (Class => CS.To_Chars_Ptr(Ch_Class'Unchecked_Access));
  end;

  procedure Register_Module_Directory
    (Path : in String)
  is
    Ch_Path : aliased C.char_array := C.To_C(Path);
  begin
    AG_RegisterModuleDirectory
      (Path => CS.To_Chars_Ptr(Ch_Path'Unchecked_Access));
  end;

  procedure Unregister_Module_Directory
    (Path : in String)
  is
    Ch_Path : aliased C.char_array := C.To_C(Path);
  begin
    AG_UnregisterModuleDirectory
      (Path => CS.To_Chars_Ptr(Ch_Path'Unchecked_Access));
  end;

  function Is_Of_Class
    (Object  : in Object_Not_Null_Access;
     Pattern : in String) return Boolean
  is
    Ch_Pattern : aliased C.char_array := C.To_C(Pattern);
  begin
    return AG_OfClass
      (Object  => Object,
       Pattern => CS.To_Chars_Ptr(Ch_Pattern'Unchecked_Access)) = 1;
  end;
  
  function Is_A
    (Object  : in Object_Not_Null_Access;
     Pattern : in String) return Boolean
  is
    Ch_Pattern : aliased C.char_array := C.To_C(Pattern);
  begin
    return AG_OfClass
      (Object  => Object,
       Pattern => CS.To_Chars_Ptr(Ch_Pattern'Unchecked_Access)) = 1;
  end;

  function In_Use (Object : in Object_Not_Null_Access) return Boolean is
  begin
    return AG_ObjectInUse(Object) = 1;
  end;

  -------------------
  -- Serialization --
  -------------------

  function Load (Object : in Object_Not_Null_Access) return Boolean is
  begin
    return AG_ObjectLoad(Object) = 0;
  end;

  function Load
    (Object : in Object_Not_Null_Access;
     File   : in String) return Boolean
  is
    Ch_File : aliased C.char_array := C.To_C(File);
  begin
    return AG_ObjectLoadFromFile
      (Object => Object,
       File   => CS.To_Chars_Ptr(Ch_File'Unchecked_Access)) = 0;
  end;

  function Load_Data (Object : in Object_Not_Null_Access) return Boolean is
  begin
    return AG_ObjectLoadData(Object) = 0;
  end;

  function Load_Data
    (Object : in Object_Not_Null_Access;
     File   : in String) return Boolean
  is
    Ch_File : aliased C.char_array := C.To_C(File);
  begin
    return AG_ObjectLoadDataFromFile
      (Object => Object,
       File   => CS.To_Chars_Ptr(Ch_File'Unchecked_Access)) = 0;
  end;

  function Load_Generic (Object : in Object_Not_Null_Access) return Boolean is
  begin
    return AG_ObjectLoadGeneric(Object) = 0;
  end;

  function Load_Generic
    (Object : in Object_Not_Null_Access;
     File   : in String) return Boolean
  is
    Ch_File : aliased C.char_array := C.To_C(File);
  begin
    return AG_ObjectLoadGenericFromFile
      (Object => Object,
       File   => CS.To_Chars_Ptr(Ch_File'Unchecked_Access)) = 0;
  end;

  function Save (Object : in Object_Not_Null_Access) return Boolean is
  begin
    return AG_ObjectSave(Object) = 0;
  end;
  
  function Save
    (Object : in Object_Not_Null_Access;
     File   : in String) return Boolean
  is
    Ch_File : aliased C.char_array := C.To_C(File);
  begin
    return AG_ObjectSaveToFile
      (Object => Object,
       File   => CS.To_Chars_Ptr(Ch_File'Unchecked_Access)) = 0;
  end;

  function Save_All (Object : in Object_Not_Null_Access) return Boolean is
  begin
    return AG_ObjectSaveAll(Object) = 0;
  end;

  function Serialize
    (Object : in Object_Not_Null_Access;
     Source : in DS.Data_Source_Not_Null_Access) return Boolean is
  begin
    return AG_ObjectSerialize(Object, Source) = 0;
  end;

  function Unserialize
    (Object : in Object_Not_Null_Access;
     Source : in DS.Data_Source_Not_Null_Access) return Boolean is
  begin
    return AG_ObjectUnserialize(Object, Source) = 0;
  end;

  function Read_Header
    (Source : in DS.Data_Source_Not_Null_Access;
     Header : in Header_Access) return Boolean is
  begin
    return AG_ObjectReadHeader (Source, Header) = 0;
  end;

  function Page_In
    (Object : in Object_Not_Null_Access) return Boolean is
  begin
    return AG_ObjectPageIn (Object) = 0;
  end;

  function Page_Out
    (Object : in Object_Not_Null_Access) return Boolean is
  begin
    return AG_ObjectPageOut (Object) = 0;
  end;
  
  ------------
  -- Events --
  ------------
  
  function Set_Event
    (Object    : in Object_Not_Null_Access;
     Event     : in String;
     Func      : in Event_Func_Access;
     Async     : in Boolean := False;
     Propagate : in Boolean := False) return Event_Not_Null_Access
  is
    Ch_Event : aliased C.char_array := C.To_C(Event);
    Result   : constant Event_Not_Null_Access := AG_SetEvent
      (Object => Object,
       Event  => CS.To_Chars_Ptr(Ch_Event'Unchecked_Access),
       Func   => Func,
       Format => CS.Null_Ptr);
  begin
    -- TODO Async, Propagate
    return (Result);
  end;
  
  procedure Set_Event
    (Object    : in Object_Not_Null_Access;
     Event     : in String;
     Func      : in Event_Func_Access;
     Async     : in Boolean := False;
     Propagate : in Boolean := False)
  is
    Ch_Event : aliased C.char_array := C.To_C(Event);
  begin
    AG_SetEvent
      (Object => Object,
       Event  => CS.To_Chars_Ptr(Ch_Event'Unchecked_Access),
       Func   => Func,
       Format => CS.Null_Ptr);
  end;
  
  function Add_Event
    (Object    : in Object_Not_Null_Access;
     Event     : in String;
     Func      : in Event_Func_Access;
     Async     : in Boolean := False;
     Propagate : in Boolean := False) return Event_Not_Null_Access
  is
    Ch_Event : aliased C.char_array := C.To_C(Event);
    Result   : constant Event_Not_Null_Access := AG_AddEvent
      (Object => Object,
       Event  => CS.To_Chars_Ptr(Ch_Event'Unchecked_Access),
       Func   => Func,
       Format => CS.Null_Ptr);
  begin
    return (Result);
  end;
  
  procedure Add_Event
    (Object    : in Object_Not_Null_Access;
     Event     : in String;
     Func      : in Event_Func_Access;
     Async     : in Boolean := False;
     Propagate : in Boolean := False)
  is
    Ch_Event : aliased C.char_array := C.To_C(Event);
  begin
    AG_AddEvent
      (Object => Object,
       Event  => CS.To_Chars_Ptr(Ch_Event'Unchecked_Access),
       Func   => Func,
       Format => CS.Null_Ptr);
  end;
  
  procedure Post_Event
    (Object : in Object_Not_Null_Access;
     Event  : in String)
  is
    Ch_Event  : aliased C.char_array := C.To_C(Event);
  begin
    AG_PostEvent
      (Object => Object,
       Event  => CS.To_Chars_Ptr(Ch_Event'Unchecked_Access),
       Format => CS.Null_Ptr);
  end;
  
  procedure Post_Event
    (Object : in Object_Not_Null_Access;
     Event  : in Event_Not_Null_Access) is
  begin
    AG_PostEventByPtr
      (Object => Object,
       Event  => Event,
       Format => CS.Null_Ptr);
  end;
  
  procedure Debug
    (Object  : in Object_Access;
     Message : in String)
  is
    Ch_Format  : aliased C.char_array := C.To_C("%s");
    Ch_Message : aliased C.char_array := C.To_C(Message);
  begin
    AG_Debug
      (Object  => Object,
       Format  => CS.To_Chars_Ptr(Ch_Format'Unchecked_Access),
       Message => CS.To_Chars_Ptr(Ch_Message'Unchecked_Access));
  end;
  
  ------------
  -- Timers --
  ------------

  function Add_Timer
    (Object   : in Object_Access;
     Timer    : in TMR.Timer_not_null_Access;
     Interval : in Interfaces.Unsigned_32;
     Func     : in TMR.Timer_Callback) return Boolean
  is
  begin
    return 0 = AG_AddTimer
      (Object   => Object,
       Timer    => Timer,
       Interval => Interval,
       Func     => Func,
       Flags    => 0,
       Format   => CS.Null_Ptr);
  end;
  
  function Add_Timer
    (Object   : in Object_Access;
     Interval : in Interfaces.Unsigned_32;
     Func     : in TMR.Timer_Callback) return TMR.Timer_Access
  is
  begin
    return AG_AddTimerAuto
      (Object   => Object,
       Interval => Interval,
       Func     => Func,
       Format   => CS.Null_Ptr);
  end;
 
  ---------------
  -- Variables --
  ---------------

  function Defined
    (Object   : in Object_not_null_Access;
     Variable : in String) return Boolean
  is
    Ch_Name : aliased C.char_array := C.To_C(Variable);
    Result  : C.int;
  begin
    Lock(Object);
    Result := AG_Defined
      (Object => Object,
       Name   => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access));
    Unlock(Object);
    return Result = 1;
  end;

  --
  -- Compare two variables with no dereference. Discrete types are compared
  -- by value. Strings are compared case-sensitively. Reference types are
  -- compared by their pointer value.
  --
  function "=" (Left, Right : in Variable_not_null_Access) return Boolean is
  begin
    return 0 = AG_CompareVariables (Left, Right);
  end;

end Agar.Object;
