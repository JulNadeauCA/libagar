with Agar;
with Agar.Object;
with Agar.Data_Source;
with Agar.Variable;
with System.Address_To_Access_Conversions;
with Interfaces.C;

--
-- Example of an Agar object class called "Animal".
--

package Animal is
  package OBJ renames Agar.Object; 
  package DS renames Agar.Data_Source; 
  package AV renames Agar.Variable; 
  package C renames Interfaces.C;
  
  use type C.int;
  use type C.C_float;
  use type C.double;

  Success : constant C.int := 0;
  Error   : constant C.int := -1;

  ------------------------------
  -- Object Class Description --
  ------------------------------
  type Ecological_Group_t is
    (Undefined, Carnivore, Herbivore, Omnivore, Detritivore, Parasite);
  type Animal_Class_t is limited record
    Class            : OBJ.Class_t;             -- Agar(Object) -> Animal
    -- more fields --
    Ecological_Group : Ecological_Group_t;
    Description      : String (1 .. 200);
  end record                                              with Convention => C;
  type Animal_Class_Access_t is access all Animal_Class_t with Convention => C;

  ---------------------
  -- Object Instance --
  ---------------------
  type Animal_t is limited record
    Object : OBJ.Object_t;                      -- Agar(Object) -> Animal
    -- more fields --
    Age    : Interfaces.Unsigned_8;
    Exp    : Interfaces.Unsigned_16;
    Name   : String (1 .. 20);
    Bio    : String (1 .. 100);
    X      : C.double;
    Y      : C.double;
    Z      : C.double;
  end record                                  with Convention => C;
  type Animal_Access_t is access all Animal_t with Convention => C;
  
  package C_cls is new System.Address_To_Access_Conversions (Animal_Class_t);
  Object_Class : OBJ.Class_Access_t   := null;    -- Generic class description
  Animal_Class : C_cls.Object_Pointer := null;    -- Our derived description
  
  function Create_Class return OBJ.Class_Not_Null_Access_t;
  procedure Destroy_Class;

  procedure Init (Object : OBJ.Object_Access_t)     with Convention => C;
  procedure Destroy (Object : OBJ.Object_Access_t)  with Convention => C;

  function Load
    (Object  : OBJ.Object_Access_t;
     Source  : DS.Data_Source_Access_t;
     Version : OBJ.Version_Access_t) return C.int   with Convention => C;
  function Save
    (Object : OBJ.Object_Access_t;
     Dest   : DS.Data_Source_Access_t) return C.int with Convention => C;
 
end Animal;
