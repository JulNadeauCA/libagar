with Agar.Core.Thin;
with Agar.Core.Data_Source;
with Interfaces;

package Agar.Core.Object is

  subtype Object_Access_t                 is Thin.Object.Object_Access_t;
  subtype Object_Not_Null_Access_t        is Thin.Object.Object_Not_Null_Access_t;
  subtype Object_Header_Access_t          is Thin.Object.Object_Header_Access_t;
  subtype Object_Header_Not_Null_Access_t is Thin.Object.Object_Header_Not_Null_Access_t;
  subtype Class_Access_t                  is Thin.Object.Class_Access_t;
  subtype Class_Not_Null_Access_t         is Thin.Object.Class_Not_Null_Access_t;
  subtype Dependency_Access_t             is Thin.Object.Dependency_Access_t;
  subtype Dependency_Not_Null_Access_t    is Thin.Object.Dependency_Not_Null_Access_t;

  --
  --
  --

  function New_Object
    (Parent       : in Object_Access_t;
     Name         : in String;
     Object_Class : in Class_Not_Null_Access_t) return Object_Access_t;

  procedure Init
    (Object       : in Object_Not_Null_Access_t;
     Object_Class : in Class_Not_Null_Access_t)
      renames Thin.Object.Init;

  procedure Init_Static
    (Object       : in Object_Not_Null_Access_t;
     Object_Class : in Class_Not_Null_Access_t)
      renames Thin.Object.Init_Static;

  procedure Attach
    (New_Parent : in Object_Access_t;
     Object     : in Object_Not_Null_Access_t)
      renames Thin.Object.Attach;

  procedure Attach_To_Named
    (VFS_Root   : in Object_Not_Null_Access_t;
     Path       : in String;
     Child      : in Object_Access_t);

  procedure Detach
    (Object : in Object_Not_Null_Access_t)
      renames Thin.Object.Detach;

  procedure Move_To_Head
    (Object : in Object_Not_Null_Access_t)
      renames Thin.Object.Move_To_Head;

  procedure Move_To_Tail
    (Object : in Object_Not_Null_Access_t)
      renames Thin.Object.Move_To_Tail;

  procedure Move_Up
    (Object : in Object_Not_Null_Access_t)
      renames Thin.Object.Move_Up;

  procedure Move_Down
    (Object : in Object_Not_Null_Access_t)
      renames Thin.Object.Move_Down;

  procedure Delete
    (Object : in Object_Not_Null_Access_t)
      renames Thin.Object.Delete;

  function Root (Object : in Object_Not_Null_Access_t)
    return Object_Not_Null_Access_t
      renames Thin.Object.Root;

  function Parent (Object : in Object_Not_Null_Access_t)
    return Object_Access_t renames Thin.Object.Parent;

  function Find
    (VFS_Root : in Object_Not_Null_Access_t;
     Pattern  : in String) return Object_Access_t;

  function Find_Parent
    (VFS_Root    : in Object_Not_Null_Access_t;
     Name        : in String;
     Object_Type : in String) return Object_Access_t;

  function Find_Child
    (VFS_Root : in Object_Not_Null_Access_t;
     Name     : in String) return Object_Access_t;

  -- XXX: Semantics?
  procedure Copy_Name
    (Object : in     Object_Not_Null_Access_t;
     Path   :    out String;
     Used   :    out Natural);

  procedure Lock (Object : in Object_Not_Null_Access_t)
    renames Thin.Object.Lock;

  procedure Unlock (Object : in Object_Not_Null_Access_t)
    renames Thin.Object.Unlock;

  procedure Lock_VFS (Object : in Object_Not_Null_Access_t)
    renames Thin.Object.Lock_VFS;

  procedure Unlock_VFS (Object : in Object_Not_Null_Access_t)
    renames Thin.Object.Unlock_VFS;

  procedure Set_Name
    (Object : in Object_Not_Null_Access_t;
     Name   : in String);

  -- XXX: Semantics?
--  procedure Generate_Name
--    (Object       : in Object_Not_Null_Access_t;
--     Object_Class : in Class_Not_Null_Access_t;
--     Buffer       : in String);
--  procedure Generate_Name_Prefixed
--    (Object : in     Object_Not_Null_Access_t;
--     Prefix : in     String;
--     Buffer :    out String;
--     Size   :    out Natural);

  -- FIXME : AG_ObjectSetDebugFn

  procedure Register_Class
    (Object_Class : Class_Not_Null_Access_t)
      renames Thin.Object.Register_Class;

  procedure Unregister_Class
    (Object_Class : Class_Not_Null_Access_t)
      renames Thin.Object.Unregister_Class;

  procedure Register_Namespace
    (Name   : in String;
     Prefix : in String;
     URL    : in String);

  procedure Unregister_Namespace
    (Name   : in String);

  function Lookup_Class
    (Spec : in String) return Class_Access_t;

  function Load_Class
    (Spec : in String) return Class_Access_t;

  procedure Register_Module_Directory
    (Path : in String);

  procedure Unregister_Module_Directory
    (Path : in String);

  function Is_Of_Class
    (Object  : in Object_Not_Null_Access_t;
     Pattern : in String) return Boolean;

  function Superclass
    (Object  : in Object_Not_Null_Access_t) return Object_Access_t
      renames Thin.Object.Superclass;

  function In_Use
    (Object  : in Object_Not_Null_Access_t) return Boolean;

  function Add_Dependency
    (Object     : in Object_Not_Null_Access_t;
     Dependency : in Object_Not_Null_Access_t;
     Persistent : in Boolean) return Dependency_Access_t;

  procedure Delete_Dependency
    (Object     : in Object_Not_Null_Access_t;
     Dependency : in Object_Not_Null_Access_t)
      renames Thin.Object.Delete_Dependency;

  function Encode_Name
    (Object     : in Object_Not_Null_Access_t;
     Dependency : in Object_Not_Null_Access_t) return Interfaces.Unsigned_32
      renames Thin.Object.Encode_Name;

  function Find_Dependency
    (Object     : in Object_Not_Null_Access_t;
     Index      : in Interfaces.Unsigned_32;
     Pointer    : access Object_Not_Null_Access_t) return Boolean;

  procedure Destroy (Object : in Object_Not_Null_Access_t)
    renames Thin.Object.Destroy;

  procedure Free_Dataset (Object : in Object_Not_Null_Access_t)
    renames Thin.Object.Free_Dataset;

  procedure Free_Events (Object : in Object_Not_Null_Access_t)
    renames Thin.Object.Free_Events;

  procedure Free_Variables (Object : in Object_Not_Null_Access_t)
    renames Thin.Object.Free_Variables;

  procedure Free_Dependencies (Object : in Object_Not_Null_Access_t)
    renames Thin.Object.Free_Dependencies;

  procedure Free_Dummy_Dependencies (Object : in Object_Not_Null_Access_t)
    renames Thin.Object.Free_Dummy_Dependencies;

  procedure Free_Children (Object : in Object_Not_Null_Access_t)
    renames Thin.Object.Free_Children;

  function Load (Object : in Object_Not_Null_Access_t) return Boolean;

  function Load_From_File
    (Object : in Object_Not_Null_Access_t;
     File   : in String) return Boolean;

  function Load_Data (Object : in Object_Not_Null_Access_t) return Boolean;

  function Load_Data_From_File
    (Object : in Object_Not_Null_Access_t;
     File   : in String) return Boolean;

  function Load_Generic (Object : in Object_Not_Null_Access_t) return Boolean;

  function Load_Generic_From_File
    (Object : in Object_Not_Null_Access_t;
     File   : in String) return Boolean;

  function Save (Object : in Object_Not_Null_Access_t) return Boolean;

  function Save_All (Object : in Object_Not_Null_Access_t) return Boolean;

  function Save_To_File
    (Object : in Object_Not_Null_Access_t;
     File   : in String) return Boolean;

  function Serialize
    (Object : in Object_Not_Null_Access_t;
     Source : in Data_Source.Data_Source_Not_Null_Access_t) return Boolean;

  function Unserialize
    (Object : in Object_Not_Null_Access_t;
     Source : in Data_Source.Data_Source_Not_Null_Access_t) return Boolean;

  function Read_Header
    (Object : in Object_Not_Null_Access_t;
     Header : in Object_Header_Access_t) return Boolean;

  function Page_In (Object : in Object_Not_Null_Access_t) return Boolean;

  function Page_Out (Object : in Object_Not_Null_Access_t) return Boolean;

end Agar.Core.Object;
