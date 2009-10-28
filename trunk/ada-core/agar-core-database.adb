with Interfaces.C;
with System.Address_To_Access_Conversions;
with System;

package body Agar.Core.Database is
  package C renames Interfaces.C;

  use type C.int;
  use type C.size_t;

  procedure New_Database
    (Database_Type : in     Type_t;
     Database      :    out Database_Access_t) is
  begin
    null;
  end New_Database;

  package body Generic_Database is

    package Data_Access_Conversion is new
      System.Address_To_Access_Conversions (Object => Data_Type);

    function Exists
      (Database : in Database_Not_Null_Access_t;
       Key      : in Key_Type) return Boolean is
    begin
      return 1 = Thin.DB.Exists_Binary_Key
        (DB       => Database,
         Key_Data => Key'Address,
         Key_Size => Key'Size / System.Storage_Unit);
    end Exists;

    procedure Lookup
      (Database       : in     Database_Not_Null_Access_t;
       Key            : in     Key_Type;
       Database_Entry :    out Entry_t;
       Found          :    out Boolean)
    is
      DB_Entry : aliased Thin.DB.Entry_t;
    begin
      Found := 0 = Thin.DB.Lookup_Binary_Key
        (DB       => Database,
         DB_Entry => DB_Entry'Unchecked_Access,
         Key_Data => Key'Address,
         Key_Size => Key'Size / System.Storage_Unit);
      if Found then
        Database_Entry.Data      := Data_Type_Access (Data_Access_Conversion.To_Pointer (DB_Entry.Data));
        Database_Entry.Data_Size := Natural (DB_Entry.Data_Size);
      else
        Database_Entry := Null_Entry;
      end if;
    end Lookup;

    function Delete
      (Database : in Database_Not_Null_Access_t;
       Key      : in Key_Type) return Boolean is
    begin
      return 0 = Thin.DB.Delete_Binary_Key
        (DB       => Database,
         Key_Data => Key'Address,
         Key_Size => Key'Size / System.Storage_Unit);
    end Delete;

    function Put
      (Database : in Database_Not_Null_Access_t;
       Key      : in Key_Type_Access;
       Data     : in Data_Type_Access) return Boolean
    is
      DB_Entry : aliased Thin.DB.Entry_t;
    begin
      DB_Entry.DB        := Database;
      DB_Entry.Key       := Key.all'Address;
      DB_Entry.Key_Size  := Key'Size / System.Storage_Unit;
      DB_Entry.Data      := Data.all'Address;
      DB_Entry.Data_Size := Data'Size / System.Storage_Unit;

      return 0 = Thin.DB.Put
        (DB       => Database,
         DB_Entry => DB_Entry'Unchecked_Access);
    end Put;

  end Generic_Database;

end Agar.Core.Database;
