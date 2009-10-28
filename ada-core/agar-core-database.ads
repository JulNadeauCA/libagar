with Agar.Core.Thin;

package Agar.Core.Database is

  subtype Database_Access_t          is Thin.DB.DB_Access_t;
  subtype Database_Not_Null_Access_t is Thin.DB.DB_Not_Null_Access_t;

  type Type_t is new Thin.DB.Type_t;

  procedure New_Database
    (Database_Type : in     Type_t;
     Database      :    out Database_Access_t);

  procedure Sync
    (Database : in Database_Not_Null_Access_t)
      renames Thin.DB.Sync;

  generic
    type Key_Type         is private;
    type Key_Type_Access  is access all Key_Type;
    type Data_Type        is private;
    type Data_Type_Access is access all Data_Type;

  package Generic_Database is

    type Entry_t is record
      Key       : Key_Type_Access;
      Key_Size  : Natural;
      Data      : Data_Type_Access;
      Data_Size : Natural;
    end record;

    Null_Entry : constant Entry_t :=
      (Key       => null,
       Key_Size  => 0,
       Data      => null,
       Data_Size => 0);

    function Exists
      (Database : in Database_Not_Null_Access_t;
       Key      : in Key_Type) return Boolean;

    procedure Lookup
      (Database       : in     Database_Not_Null_Access_t;
       Key            : in     Key_Type;
       Database_Entry :    out Entry_t;
       Found          :    out Boolean);

    function Delete
      (Database : in Database_Not_Null_Access_t;
       Key      : in Key_Type) return Boolean;

    function Put
      (Database : in Database_Not_Null_Access_t;
       Key      : in Key_Type_Access;
       Data     : in Data_Type_Access) return Boolean;

  end Generic_Database;

end Agar.Core.Database;
