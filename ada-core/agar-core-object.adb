with C_String;
with Interfaces.C;
with Interfaces.C.Strings;

package body Agar.Core.Object is
  package C         renames Interfaces.C;
  package C_Strings renames Interfaces.C.Strings;

  use type C.int;

  function New_Object
    (Parent       : in Object_Access_t;
     Name         : in String;
     Object_Class : in Class_Not_Null_Access_t) return Object_Access_t
  is
    Ch_Name : aliased C.char_array := C.To_C (Name);
  begin
    return Thin.Object.New_Object
      (Parent       => Parent,
       Name         => C_String.To_C_String (Ch_Name'Unchecked_Access),
       Object_Class => Object_Class);
  end New_Object;

  procedure Attach_To_Named
    (VFS_Root   : in Object_Not_Null_Access_t;
     Path       : in String;
     Child      : in Object_Access_t)
  is
    Ch_Path : aliased C.char_array := C.To_C (Path);
  begin
    Thin.Object.Attach_To_Named
      (VFS_Root => VFS_Root,
       Path     => C_String.To_C_String (Ch_Path'Unchecked_Access),
       Child    => Child);
  end Attach_To_Named;

  function Find
    (VFS_Root : in Object_Not_Null_Access_t;
     Pattern  : in String) return Object_Access_t
  is
    Ch_Format  : aliased C.char_array := C.To_C ("%s");
    Ch_Pattern : aliased C.char_array := C.To_C (Pattern);
  begin
    return Thin.Object.Find
      (VFS_Root => VFS_Root,
       Format   => C_String.To_C_String (Ch_Format'Unchecked_Access),
       Data     => C_String.To_C_String (Ch_Pattern'Unchecked_Access));
  end Find;

  function Find_Parent
    (VFS_Root    : in Object_Not_Null_Access_t;
     Name        : in String;
     Object_Type : in String) return Object_Access_t
  is
    Ch_Name        : aliased C.char_array := C.To_C (Name);
    Ch_Object_Type : aliased C.char_array := C.To_C (Object_Type);
  begin
    return Thin.Object.Find_Parent
      (VFS_Root    => VFS_Root,
       Name        => C_String.To_C_String (Ch_Name'Unchecked_Access),
       Object_Type => C_String.To_C_String (Ch_Object_Type'Unchecked_Access));
  end Find_Parent;

  function Find_Child
    (VFS_Root : in Object_Not_Null_Access_t;
     Name     : in String) return Object_Access_t
  is
    Ch_Name : aliased C.char_array := C.To_C (Name);
  begin
    return Thin.Object.Find_Child
      (VFS_Root => VFS_Root,
       Name     => C_String.To_C_String (Ch_Name'Unchecked_Access));
  end Find_Child;

  procedure Copy_Name
    (Object : in     Object_Not_Null_Access_t;
     Path   :    out String;
     Used   :    out Natural)
  is
    Path_Length : constant C.size_t                    := Path'Length;
    Ch_Path     : aliased  C.char_array                := (Path_Length => C.nul);
    Ch_Path_Acc : constant C_Strings.char_array_access := Ch_Path'Unchecked_Access;
    Result      : C.int;
    Length      : C.size_t;
  begin
    Result := Thin.Object.Copy_Name
      (Object => Object,
       Path   => C_String.To_C_Char_Array (Ch_Path_Acc),
       Size   => Ch_Path'Length);
    if Result = 0 then
      Length := C_String.Length (C_String.To_C_String (Ch_Path_Acc));
      Path (Path'First .. Path'First + Natural (Length)) :=
        C_String.To_String
          (Item => C_String.To_C_Char_Array (Ch_Path_Acc),
           Size => Length);
      Used := Natural (Length);
    else
      Path (Path'First) := Character'Val (0);
      Used              := 0;
    end if;
  end Copy_Name;

  procedure Set_Name
    (Object : in Object_Not_Null_Access_t;
     Name   : in String)
  is
    Ch_Format : aliased C.char_array := C.To_C ("%s");
    Ch_Name   : aliased C.char_array := C.To_C (Name);
  begin
    Thin.Object.Set_Name
      (Object => Object,
       Format => C_String.To_C_String (Ch_Format'Unchecked_Access),
       Data   => C_String.To_C_String (Ch_Name'Unchecked_Access));
  end Set_Name;

  procedure Register_Namespace
    (Name   : in String;
     Prefix : in String;
     URL    : in String)
  is
    Ch_Name   : aliased C.char_array := C.To_C (Name);
    Ch_Prefix : aliased C.char_array := C.To_C (Prefix);
    Ch_URL    : aliased C.char_array := C.To_C (URL);
  begin
    Thin.Object.Register_Namespace
      (Name   => C_String.To_C_String (Ch_Name'Unchecked_Access),
       Prefix => C_String.To_C_String (Ch_Prefix'Unchecked_Access),
       URL    => C_String.To_C_String (Ch_URL'Unchecked_Access));
  end Register_Namespace;

  procedure Unregister_Namespace
    (Name   : in String)
  is
    Ch_Name   : aliased C.char_array := C.To_C (Name);
  begin
    Thin.Object.Unregister_Namespace (C_String.To_C_String (Ch_Name'Unchecked_Access));
  end Unregister_Namespace;

  function Lookup_Class
    (Spec : in String) return Class_Access_t
  is
    Ch_Spec : aliased C.char_array := C.To_C (Spec);
  begin
    return Thin.Object.Lookup_Class (C_String.To_C_String (Ch_Spec'Unchecked_Access));
  end Lookup_Class;

  function Load_Class
    (Spec : in String) return Class_Access_t
  is
    Ch_Spec : aliased C.char_array := C.To_C (Spec);
  begin
    return Thin.Object.Load_Class (C_String.To_C_String (Ch_Spec'Unchecked_Access));
  end Load_Class;

  procedure Register_Module_Directory
    (Path : in String)
  is
    Ch_Path : aliased C.char_array := C.To_C (Path);
  begin
    Thin.Object.Register_Module_Directory
      (C_String.To_C_String (Ch_Path'Unchecked_Access));
  end Register_Module_Directory;

  procedure Unregister_Module_Directory
    (Path : in String)
  is
    Ch_Path : aliased C.char_array := C.To_C (Path);
  begin
    Thin.Object.Unregister_Module_Directory
      (C_String.To_C_String (Ch_Path'Unchecked_Access));
  end Unregister_Module_Directory;

  function Is_Of_Class
    (Object  : in Object_Not_Null_Access_t;
     Pattern : in String) return Boolean
  is
    Ch_Pattern : aliased C.char_array := C.To_C (Pattern);
  begin
    return 1 = Thin.Object.Is_Of_Class
      (Object  => Object,
       Pattern => C_String.To_C_String (Ch_Pattern'Unchecked_Access));
  end Is_Of_Class;

  function In_Use
    (Object  : in Object_Not_Null_Access_t) return Boolean is
  begin
    return 1 = Thin.Object.In_Use (Object);
  end In_Use;

  function Add_Dependency
    (Object     : in Object_Not_Null_Access_t;
     Dependency : in Object_Not_Null_Access_t;
     Persistent : in Boolean) return Dependency_Access_t is
  begin
    return Thin.Object.Add_Dependency
      (Object     => Object,
       Dependency => Dependency,
       Persistent => Boolean'Pos (Persistent));
  end Add_Dependency;

  function Find_Dependency
    (Object     : in Object_Not_Null_Access_t;
     Index      : in Interfaces.Unsigned_32;
     Pointer    : access Object_Not_Null_Access_t) return Boolean is
  begin
    return 0 = Thin.Object.Find_Dependency
      (Object  => Object,
       Index   => Index,
       Pointer => Pointer);
  end Find_Dependency;

  function Load (Object : in Object_Not_Null_Access_t) return Boolean is
  begin
    return 0 = Thin.Object.Load (Object);
  end Load;

  function Load_From_File
    (Object : in Object_Not_Null_Access_t;
     File   : in String) return Boolean
  is
    Ch_File : aliased C.char_array := C.To_C (File);
  begin
    return 0 = Thin.Object.Load_From_File
      (Object => Object,
       File   => C_String.To_C_String (Ch_File'Unchecked_Access));
  end Load_From_File;

  function Load_Data (Object : in Object_Not_Null_Access_t) return Boolean is
  begin
    return 0 = Thin.Object.Load_Data (Object);
  end Load_Data;

  function Load_Data_From_File
    (Object : in Object_Not_Null_Access_t;
     File   : in String) return Boolean
  is
    Ch_File : aliased C.char_array := C.To_C (File);
  begin
    return 0 = Thin.Object.Load_Data_From_File
      (Object => Object,
       File   => C_String.To_C_String (Ch_File'Unchecked_Access));
  end Load_Data_From_File;

  function Load_Generic (Object : in Object_Not_Null_Access_t) return Boolean is
  begin
    return 0 = Thin.Object.Load_Generic (Object);
  end Load_Generic;

  function Load_Generic_From_File
    (Object : in Object_Not_Null_Access_t;
     File   : in String) return Boolean
  is
    Ch_File : aliased C.char_array := C.To_C (File);
  begin
    return 0 = Thin.Object.Load_Generic_From_File
      (Object => Object,
       File   => C_String.To_C_String (Ch_File'Unchecked_Access));
  end Load_Generic_From_File;

  function Save (Object : in Object_Not_Null_Access_t) return Boolean is
  begin
    return 0 = Thin.Object.Save (Object);
  end Save;

  function Save_All (Object : in Object_Not_Null_Access_t) return Boolean is
  begin
    return 0 = Thin.Object.Save_All (Object);
  end Save_All;

  function Save_To_File
    (Object : in Object_Not_Null_Access_t;
     File   : in String) return Boolean
  is
    Ch_File : aliased C.char_array := C.To_C (File);
  begin
    return 0 = Thin.Object.Save_To_File
      (Object => Object,
       File   => C_String.To_C_String (Ch_File'Unchecked_Access));
  end Save_To_File;

  function Serialize
    (Object : in Object_Not_Null_Access_t;
     Source : in Data_Source.Data_Source_Not_Null_Access_t) return Boolean is
  begin
    return 0 = Thin.Object.Serialize (Object, Source);
  end Serialize;

  function Unserialize
    (Object : in Object_Not_Null_Access_t;
     Source : in Data_Source.Data_Source_Not_Null_Access_t) return Boolean is
  begin
    return 0 = Thin.Object.Unserialize (Object, Source);
  end Unserialize;

  function Read_Header
    (Object : in Object_Not_Null_Access_t;
     Header : in Object_Header_Access_t) return Boolean is
  begin
    return 0 = Thin.Object.Read_Header (Object, Header);
  end Read_Header;

  function Page_In (Object : in Object_Not_Null_Access_t) return Boolean is
  begin
    return 0 = Thin.Object.Page_In (Object);
  end Page_In;

  function Page_Out (Object : in Object_Not_Null_Access_t) return Boolean is
  begin
    return 0 = Thin.Object.Page_Out (Object);
  end Page_Out;

end Agar.Core.Object;
