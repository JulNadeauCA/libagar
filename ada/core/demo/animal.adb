with Ada.Text_IO;

----------------------------
-- Agar(Object) -> Animal --
----------------------------

--
-- Ada implementation of the Agar object class "Animal".
--

package body Animal is
  package C_obj is new System.Address_To_Access_Conversions (Animal);
  package T_IO renames Ada.Text_IO;

  function Create_Class return OBJ.Class_Not_Null_Access is
  begin
    --
    -- Register our "Animal" class with the Agar object system.
    --
    Generic_Object_Class := OBJ.Create_Class
      (Hierarchy    => "Animal",
       Object_Size  => Animal'Size,
       Class_Size   => Animal_Class'Size,
       Major        => 1,
       Minor        => 2,
       Init_Func    => Init'Access,
       Destroy_Func => Destroy'Access,
       Load_Func    => Load'Access,
       Save_Func    => Save'Access);
    --
    -- Initialize our derived class description structure. This will be
    -- shared between all instances of Animal.
    --
    Animal_Object_Class := C_cls.To_Pointer(Generic_Object_Class.all'Address);
    Animal_Object_Class.Ecological_Group := Undefined;
    Animal_Object_Class.Description := (others => '_');

    return Generic_Object_Class;
  end;

  procedure Destroy_Class is
  begin
    OBJ.Destroy_Class (Generic_Object_Class);
    Generic_Object_Class := null;
    Animal_Object_Class := null;
  end;

  --
  -- Initialize an instance of the Animal class.
  --
  procedure Init (Object : OBJ.Object_Access)
  is
    Ani : constant C_obj.Object_Pointer := C_obj.To_Pointer(Object.all'Address);
  begin
    T_IO.Put_Line("Animal init");
    Ani.Age := 123;
    Ani.Exp := 11111;
    Ani.Name := "Here is Ada string! ";
    Ani.Bio := (others => 'x');
    Ani.X := 3.14159265358979323846;
    Ani.Y := 3.40282346638528860e+38;
    Ani.Z := -1.0;
  end;

  --
  -- Release all resources allocated by an instance of Animal.
  --
  procedure Destroy (Object : OBJ.Object_Access)
  is begin
    T_IO.Put_Line("Animal destroy");
  end;
  
  --
  -- Serialize an Animal to a machine-independent format.
  --
  function Save
    (Object : OBJ.Object_Access;
     Dest   : DS.Data_Source_Access) return C.int
  is
    Ani : constant C_obj.Object_Pointer := C_obj.To_Pointer(Object.all'Address);
  begin
    DS.Write_Unsigned_8 (Dest, Ani.Age);
    DS.Write_Unsigned_16 (Dest, Ani.Exp);
    DS.Write_Padded_String (Dest, Ani.Name, 40);
    DS.Write_String (Dest, Ani.Bio);
    DS.Write_Double (Dest, Ani.X);
    DS.Write_Double (Dest, Ani.Y);
    DS.Write_Double (Dest, Ani.Z);

    return Success;
  end;

  --
  -- Deserialize an Animal from machine-independent format.
  --
  function Load
    (Object  : OBJ.Object_Access;
     Source  : DS.Data_Source_Access;
     Version : OBJ.Version_Access) return C.int
  is
    Ani : constant C_obj.Object_Pointer := C_obj.To_Pointer(Object.all'Address);
  begin
    Ani.Age  := DS.Read_Unsigned_8 (Source);
    Ani.Exp  := DS.Read_Unsigned_16 (Source);
    Ani.Name := DS.Read_Padded_String (Source, 40);
    Ani.Bio  := DS.Read_String (Source);
    Ani.X    := DS.Read_Double (Source);
    Ani.Y    := DS.Read_Double (Source);
    Ani.Z    := DS.Read_Double (Source);

    return Success;
  end;

end Animal;
