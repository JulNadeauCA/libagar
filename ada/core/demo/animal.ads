with Agar;
with Agar.Object;
with Agar.Data_Source;
with System.Address_To_Access_Conversions;
with Interfaces.C;

--
-- Example of an Agar object class called "Animal".
--

package Animal is
  package OBJ renames Agar.Object; 
  package DS renames Agar.Data_Source; 
  package C renames Interfaces.C;
  
  use type C.int;
  use type C.C_float;
  use type C.double;

  Success : constant C.int := 0;
  Error   : constant C.int := -1;

  -----------------------
  -- Class Description --
  -----------------------

  type Ecological_Group_t is
    (Undefined, Carnivore, Herbivore, Omnivore, Detritivore, Parasite);

  type Animal_Class is limited record
    Class            : OBJ.Class;             -- Agar(Object) -> Animal
    -- more fields --
    Ecological_Group : Ecological_Group_t;
    Description      : String (1 .. 200);
  end record
    with Convention => C;
  type Animal_Class_Access is access all Animal_Class
    with Convention => C;

  ---------------------
  -- Object Instance --
  ---------------------
  type Animal is limited record
    Object : OBJ.Object;                      -- Agar(Object) -> Animal
    -- more fields --
    Age    : Interfaces.Unsigned_8;
    Exp    : Interfaces.Unsigned_16;
    Name   : String (1 .. 20);
    Bio    : String (1 .. 100);
    X      : C.double;
    Y      : C.double;
    Z      : C.double;
  end record
    with Convention => C;

  type Animal_Access is access all Animal with Convention => C;
  
  package C_cls is new System.Address_To_Access_Conversions (Animal_Class);

  Generic_Object_Class : OBJ.Class_Access := null;
  Animal_Object_Class  : C_cls.Object_Pointer := null;
  
  function Create_Class return OBJ.Class_Not_Null_Access;
  procedure Destroy_Class;

  procedure Init (Object : OBJ.Object_Access)     with Convention => C;
  procedure Destroy (Object : OBJ.Object_Access)  with Convention => C;

  function Load
    (Object  : OBJ.Object_Access;
     Source  : DS.Data_Source_Access;
     Version : OBJ.Version_Access) return C.int   with Convention => C;

  function Save
    (Object : OBJ.Object_Access;
     Dest   : DS.Data_Source_Access) return C.int with Convention => C;
 
end Animal;
