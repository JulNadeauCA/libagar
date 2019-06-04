------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                             A G A R . D S O                              --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Ada.Containers.Indefinite_Vectors;
with Interfaces.C;
with Interfaces.C.Strings;
with Interfaces.C.Pointers;
with System;

--
-- Interface to dynamic linker facilities.
--

package Agar.DSO is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;
  
  type DSO is array (1 .. $SIZEOF_AG_DSO) of aliased Interfaces.Unsigned_8
    with Convention => C;
  for DSO'Size use $SIZEOF_AG_DSO * System.Storage_Unit;

  type DSO_Access             is access all DSO with Convention => C;
  subtype DSO_Not_Null_Access is not null DSO_Access;
  type Symbol_Access is access all System.Address with Convention => C;

  --
  -- Load the named module into the process address space. If it already
  -- exists in memory, return an access to the existing object and increment
  -- its reference count.
  --
  function Load (Name : in String) return DSO_Access;

  --
  -- Decrement the reference count of the given loaded DSO, and unload the
  -- module from the process's address space if it reaches zero.
  --
  function Unload (DSO : DSO_Not_Null_Access) return Boolean;

  --
  -- Return a handle to an already loaded DSO by name. If no such module is
  -- loaded, return NULL.
  --
  function Lookup (Name : in String) return DSO_Access;
  
  --
  -- Acquire the mutex protecting the DSO handle.
  --
  procedure Lock   with Import, Convention => C, Link_Name => "ag_lock_dso";
  procedure Unlock with Import, Convention => C, Link_Name => "ag_unlock_dso";

  --
  -- Scan the registered module directories for loadable shared libraries
  -- and return a list of String for all available modules.
  --
  package DSO_List_Package is new Ada.Containers.Indefinite_Vectors
    (Index_Type   => Positive,
     Element_Type => String);
  subtype DSO_List is DSO_List_Package.Vector;

  function Get_List return DSO_List;

  generic
    type Subprogram_Access_Type is private;

  function Symbol_Lookup
    (DSO    : in DSO_Not_Null_Access;
     Symbol : in String) return Subprogram_Access_Type;

  private

  function AG_LoadDSO
    (Name  : in CS.chars_ptr;
     Flags : C.unsigned) return DSO_Access
    with Import, Convention => C, Link_Name => "AG_LoadDSO";
  
  function AG_UnloadDSO (DSO : in DSO_Not_Null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_UnloadDSO";
  
  function AG_LookupDSO
    (Name : in CS.chars_ptr) return DSO_Access
    with Import, Convention => C, Link_Name => "AG_LookupDSO";
  
  function AG_SymDSO
    (DSO    : in DSO_Not_Null_Access;
     Symbol : in CS.chars_ptr;
     Value  : in Symbol_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_SymDSO";
    
  type DSO_List_Entry is array (C.unsigned range <>) of aliased CS.chars_ptr
    with Convention => C;
  package DSO_List_To_Strings is new Interfaces.C.Pointers
    (Index              => C.unsigned,
     Element            => CS.chars_ptr,
     Element_Array      => DSO_List_Entry,
     Default_Terminator => Null);
  
  function AG_GetDSOList
    (Count : access C.unsigned) return DSO_List_To_Strings.Pointer
    with Import, Convention => C, Link_Name => "AG_GetDSOList";
 
  procedure AG_FreeDSOList
    (List  : DSO_List_To_Strings.Pointer;
     Count : C.unsigned)
    with import, Convention => C, Link_Name => "AG_FreeDSOList";

end Agar.DSO;
