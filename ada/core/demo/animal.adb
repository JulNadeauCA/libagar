with Ada.Text_IO;

----------------------------
-- Agar(Object) -> Animal --
----------------------------

--
-- Ada implementation of the Agar object class "Animal". We derive both
-- Object_t into Animal_t and Class_t into Animal_Class_t.
--

package body Animal is
  package C_obj is new System.Address_To_Access_Conversions (Animal_t);
  package T_IO renames Ada.Text_IO;

  function Create_Class return OBJ.Class_Not_Null_Access_t is
  begin
    --
    -- Register our "Animal" class with the Agar object system.
    --
    Object_Class := OBJ.Create_Class
      (Hierarchy    => "Animal",
       Object_Size  => Animal.Animal_t'Size,
       Class_Size   => Animal.Animal_Class_t'Size,
       Major        => 1,
       Minor        => 2,
       Init_Func    => Animal.Init'Access,
       Destroy_Func => Animal.Destroy'Access,
       Load_Func    => Animal.Load'Access,
       Save_Func    => Animal.Save'Access);
    --
    -- Initialize our derived class description structure. This will be
    -- shared between all instances of Animal_t.
    --
    Animal_Class := C_cls.To_Pointer(Object_Class.all'Address);
    Animal_Class.Ecological_Group := Undefined;
    Animal_Class.Description := (others => '_');

    return Object_Class;
  end;

  procedure Destroy_Class is
  begin
    OBJ.Destroy_Class (Object_Class);
    Object_Class := null;
    Animal_Class := null;
  end;

  --
  -- Initialize an instance of the Animal_t class.
  --
  procedure Init (Object : OBJ.Object_Access_t)
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
  -- Release all resources allocated by an instance of Animal_t.
  --
  procedure Destroy (Object : OBJ.Object_Access_t)
  is begin
    T_IO.Put_Line("Animal destroy");
  end;
  
  --
  -- Serialize an Animal_t to a machine-independent format.
  --
  function Save
    (Object : OBJ.Object_Access_t;
     Dest   : DS.Data_Source_Access_t) return C.int
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
  -- Deserialize an Animal_t from machine-independent format.
  --
  function Load
    (Object  : OBJ.Object_Access_t;
     Source  : DS.Data_Source_Access_t;
     Version : OBJ.Version_Access_t) return C.int
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
