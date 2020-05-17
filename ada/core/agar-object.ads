------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                          A G A R . O B J E C T                           --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Agar.Data_Source;
with Agar.Event;
with Agar.Timer;
with Agar.Types; use Agar.Types;
with Interfaces;
with Interfaces.C;
with Interfaces.C.Strings;
with System;

--
-- Interface to the Agar Object System. This defines the base type, Agar.Object.
-- It maps transparently to AG_Object(3) in C. This allows new Agar object
-- classes to be written in Ada (and subclasses of Ada-implemented classes
-- to be written also in Ada, or again in C).
--
-- In C, Agar objects are structs derived from AG_Object. Similarly in Ada,
-- Agar object instances are limited records which derive from Agar.Object.
--
-- Shared, class-wide data is represented by limited records which derive
-- from Agar.Object.Class (which mirrors AG_ObjectClass in C).
--

package Agar.Object is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;
  package DS renames Agar.Data_Source;
  package EV renames Agar.Event;
  package TMR renames Agar.Timer;
  
  NAME_MAX      : constant Natural := $AG_OBJECT_NAME_MAX;
  HIERARCHY_MAX : constant Natural := $AG_OBJECT_HIER_MAX;
  TYPE_MAX      : constant Natural := $AG_OBJECT_TYPE_MAX;
  LIBRARIES_MAX : constant Natural := $AG_OBJECT_LIBS_MAX;

  ----------------
  -- Base Types --
  ----------------
  type Signed_8 is range -127 .. 127 with Convention => C;
  for Signed_8'Size use 8;
  type Signed_16 is range -(2 **15) .. +(2 **15 - 1) with Convention => C;
  for Signed_16'Size use 16;
  type Signed_32 is range -(2 **31) .. +(2 **31 - 1) with Convention => C;
  for Signed_32'Size use 32;
  type Signed_64 is range -(2 **63) .. +(2 **63 - 1) with Convention => C;
  for Signed_64'Size use 64;
  
  subtype Unsigned_8  is Interfaces.Unsigned_8;
  subtype Unsigned_16 is Interfaces.Unsigned_16;
  subtype Unsigned_32 is Interfaces.Unsigned_32;
  subtype Unsigned_64 is Interfaces.Unsigned_64;

#if HAVE_FLOAT
  subtype Float       is Interfaces.C.C_float;
  subtype Double      is Interfaces.C.double;
#end if;

  --------------------------
  -- Agar Object variable --
  --------------------------
  type Variable is array (1 .. $SIZEOF_AG_Variable) of
    aliased Interfaces.Unsigned_8 with Convention => C;
  for Variable'Size use $SIZEOF_AG_Variable * System.Storage_Unit;

  type Variable_Access is access all Variable with Convention => C;
  subtype Variable_not_null_Access is not null Variable_Access;
  
  -------------------------------
  -- Serialized Object version --
  -------------------------------
  type Version_t is record
    Major : Interfaces.Unsigned_32;
    Minor : Interfaces.Unsigned_32;
#if AG_MODEL = AG_MEDIUM
    C_Pad : Interfaces.Unsigned_32;
#end if;
  end record
    with Convention => C;
  type Version_Access is access all Version_t
    with Convention => C;
  
  --------------------------
  -- Agar Object Instance --
  --------------------------
  type Object;
  type Object_Access is access all Object with Convention => C;
  subtype Object_not_null_Access is not null Object_Access;
  
  type Object_Tag is array (1 .. 8) of aliased c.char with Convention => C;
  type Object_Name is array (1 .. NAME_MAX) of
    aliased c.char with Convention => C;
  
  type Class;
  type Class_Access is access all Class with Convention => C;
  subtype Class_not_null_Access is not null Class_Access;

  type Event_List is limited record
    First : EV.Event_Access;
    Last  : access EV.Event_Access;
  end record
    with Convention => C;
  type Timer_List is limited record
    First : TMR.Timer_Access;
    Last  : access TMR.Timer_Access;
  end record
    with Convention => C;
  type Variable_List is limited record
    First : Variable_Access;
    Last  : access Variable_Access;
  end record
    with Convention => C;
  type Children_List is limited record
    First : Object_Access;
    Last  : access Object_Access;
  end record
    with Convention => C;
  type Entry_in_Parent_t is limited record
    Next : Object_Access;
    Prev : access Object_Access;
  end record
    with Convention => C;
  type Entry_in_TimerQ_t is limited record
    Next : Object_Access;
    Prev : access Object_Access;
  end record
    with Convention => C;

  type Object_Mutex is array (1 .. $SIZEOF_AG_MUTEX) of
    aliased Interfaces.Unsigned_8 with Convention => C;
  for Object_Mutex'Size use $SIZEOF_AG_MUTEX * System.Storage_Unit;
  
  OBJECT_FLOATING_VARIABLES    : constant C.unsigned := 16#0_0001#;
  OBJECT_NON_PERSISTENT        : constant C.unsigned := 16#0_0002#;
  OBJECT_INDESTRUCTIBLE        : constant C.unsigned := 16#0_0004#;
  OBJECT_RESIDENT              : constant C.unsigned := 16#0_0008#;
  OBJECT_PRESERVE_DEPENDENCIES : constant C.unsigned := 16#0_0010#;
  OBJECT_STATIC                : constant C.unsigned := 16#0_0020#;
  OBJECT_READ_ONLY             : constant C.unsigned := 16#0_0040#;
  OBJECT_WAS_RESIDENT          : constant C.unsigned := 16#0_0080#;
  OBJECT_REOPEN_ON_LOAD        : constant C.unsigned := 16#0_0200#;
  OBJECT_REMAIN_DATA           : constant C.unsigned := 16#0_0400#;
  OBJECT_DEBUG                 : constant C.unsigned := 16#0_0800#;
  OBJECT_NAME_ON_ATTACH        : constant C.unsigned := 16#0_1000#;
  OBJECT_CHILD_AUTO_SAVE       : constant C.unsigned := 16#0_2000#;
  OBJECT_DEBUG_DATA            : constant C.unsigned := 16#0_4000#;
  OBJECT_IN_ATTACH_ROUTINE     : constant C.unsigned := 16#0_8000#;
  OBJECT_IN_DETACH_ROUTINE     : constant C.unsigned := 16#1_0000#;
  OBJECT_BOUND_EVENTS          : constant C.unsigned := 16#2_0000#;

  type Object is limited record
#if AG_TYPE_SAFETY
    Tag             : Object_Tag;
#end if;
    Name            : Object_Name;
    Flags           : C.unsigned;
    Class           : Class_not_null_Access;
    Events          : Event_List;
#if AG_TIMERS
    Timers          : Timer_List;
#end if;
    Variables       : Variable_List;
    Children        : Children_List;
    Entry_in_Parent : Entry_in_Parent_t;
    Parent          : Object_Access;
    Root            : Object_Access;
    Entry_in_TimerQ : Entry_in_TimerQ_t;
    Lock            : Object_Mutex;
  end record
    with Convention => C;
    
  -------------------------------------
  -- Accesses to Base Object Methods --
  -------------------------------------
  type Init_Func_Access is access procedure (Object : Object_Access)
    with Convention => C;
  type Reset_Func_Access is access procedure (Object : Object_Access)
    with Convention => C;
  type Destroy_Func_Access is access procedure (Object : Object_Access)
    with Convention => C;
  type Load_Func_Access is access function
    (Object  : Object_Access;
     Source  : DS.Data_Source_Access;
     Version : Version_Access) return C.int with Convention => C;
  type Save_Func_Access is access function
    (Object : Object_Access;
     Dest   : DS.Data_Source_Access) return C.int with Convention => C;
  type Edit_Func_Access is access function
    (Object : Object_Access) return System.Address with Convention => C;

  ------------------------------
  -- Object Class Description --
  ------------------------------
  type Class_Name is array (1 .. TYPE_MAX) of aliased c.char
    with Convention => C;
  type Class_Hierarchy is array (1 .. HIERARCHY_MAX) of aliased c.char
    with Convention => C;
  type Class_Private is array (1 .. $SIZEOF_AG_ObjectClassPvt) of
    aliased Interfaces.Unsigned_8 with Convention => C;
  for Class_Private'Size use $SIZEOF_AG_ObjectClassPvt * System.Storage_Unit;
  type Class_Private_Access is access all Class_Private with Convention => C;

  type Class is limited record
    Hierarchy    : Class_Hierarchy;
    Size         : AG_Size;
    Version      : Version_t;
    Init_Func    : Init_Func_Access;
    Reset_Func   : Reset_Func_Access;
    Destroy_Func : Destroy_Func_Access;
    Load_Func    : Load_Func_Access;
    Save_Func    : Save_Func_Access;
    Edit_Func    : Edit_Func_Access;
    -- Generated fields --
    Name         : Class_Name;
    Superclass   : Class_Access;
    Private_Data : Class_Private;
  end record
    with Convention => C;
 
  ---------------------------------
  -- Serialized Object Signature --
  ---------------------------------
  type Object_Header is array (1 .. $SIZEOF_AG_ObjectHeader) of
    aliased Interfaces.Unsigned_8 with Convention => C;
  for Object_Header'Size use $SIZEOF_AG_ObjectHeader * System.Storage_Unit;
  type Header_Access is access all Object_Header with Convention => C;
  subtype Header_not_null_Access is not null Header_Access;
  
  -----------------------
  -- Virtual Functions --
  -----------------------
  type Function_Access is access all EV.Event with Convention => C;
  subtype Function_not_null_Access is not null Function_Access;

  type Event_Func_Access is not null access procedure
    (Event : EV.Event_Access)
    with Convention => C;

  --
  -- Create a new instance of Class under Parent with the given name. Fail
  -- and return null if an object of the same name already exists.
  --
  function New_Object
    (Parent : in Object_Access;
     Name   : in String;
     Class  : in Class_not_null_Access) return Object_Access;
  --
  -- Create a new instance of Class under Parent, without naming the object.
  --
  function New_Object
    (Parent : in Object_Access;
     Class  : in Class_not_null_Access) return Object_Access;
  --
  -- Create a new instance of Class not attached to any parent.
  --
  function New_Object
    (Class : in Class_not_null_Access) return Object_Access;
   
  --
  -- Initialize an Agar Object. Used internally by New_Object.
  --
  -- Static         : Object is not (agar-)freeable autoallocated memory.
  -- Name_On_Attach : Auto-generate a unique name in Attach().
  --
  procedure Init_Object
    (Object         : in Object_not_null_Access;
     Class          : in Class_not_null_Access;
     Static         : in Boolean := False;
     Name_On_Attach : in Boolean := False);

  --
  -- Make Object a child of Parent.
  --
  procedure Attach
    (Parent : in Object_Access;
     Child  : in Object_not_null_Access);

  --
  -- Detach Object from its current Parent (if any).
  --
  procedure Detach (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ObjectDetach";

  --
  -- Change an object's position within its parent's list of child objects.
  -- This list is ordered and the order is preserved by serialization.
  --
  procedure Move_Up (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ObjectMoveUp";
  procedure Move_Down (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ObjectMoveDown";
  procedure Move_To_Head (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ObjectMoveToHead";
  procedure Move_To_Tail (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ObjectMoveToTail";

  -- Shorthand for Detach and Destroy.
  --procedure Delete (Object : in Object_not_null_Access)
  --  with Import, Convention => C, Link_Name => "AG_ObjectDelete";

  --
  -- Return the root object of the VFS that Object is part of.
  -- May return the Object itself if Object is the VFS root.
  --
  function Root (Object : in Object_not_null_Access) return Object_not_null_Access
    with Import, Convention => C, Link_Name => "AG_ObjectRoot";

  --
  -- Return an access to the Parent of an Object (which can be Null).
  --
  function Parent (Object : in Object_not_null_Access) return Object_Access
    with Import, Convention => C, Link_Name => "AG_ObjectParent";

  --
  -- Lookup an object using an absolute path name (relative to the root of
  -- the VFS which has object Root as its root).
  --
  function Find
    (Root : in Object_not_null_Access;
     Path : in String) return Object_Access;

  --
  -- Search for a parent object matching Name and Object_Type.
  --
  function Find_Parent
    (Object : in Object_not_null_Access;
     Name   : in String;
     Class  : in String) return Object_Access;

  --
  -- Return Parent's first immediate child object called Name.
  --
  function Find_Child
    (Parent : in Object_not_null_Access;
     Name   : in String) return Object_Access;

  --
  -- Return the fully qualified path name of Object (within its parent VFS).
  -- The "/" character is used as path separator.
  --
  function Get_Name (Object : in Object_not_null_Access) return String;
  
  --
  -- Acquire or release the general-purpose mutex lock protecting Object's data.
  -- This lock is always held implicitely during event processing.
  --
  procedure Lock (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "ag_object_lock";
  procedure Unlock (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "ag_object_unlock";
 
  --
  -- Acquire or release the mutex lock protecting the entire VFS.
  --
  procedure Lock_VFS (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "ag_lock_vfs";
  procedure Unlock_VFS(Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "ag_unlock_vfs";

  --
  -- Rename the object. Does not check for potential conflict.
  --
  procedure Set_Name
    (Object : in Object_not_null_Access;
     Name   : in String);

  --
  -- Return a unique (relative to its Parent) name for the Object.
  -- With a class argument, use the form : "<Class name> #123"
  -- With a Prefix string, use the form  : "<Prefix> #123"
  --
  function Generate_Name
    (Object : in Object_not_null_Access;
     Class  : in Class_not_null_Access) return String;
  function Generate_Name
    (Object : in Object_not_null_Access;
     Prefix : in String) return String;

  --
  -- Register a new class given a class description.
  --
  procedure Register_Class (Class : Class_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_RegisterClass";

  --
  -- Delete an existing class.
  --
  procedure Unregister_Class (Class : Class_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_UnregisterClass";

  --
  -- Register a new Agar object class.
  --
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
     Edit_Func    : in Edit_Func_Access := null) return Class_not_null_Access;
 
  --
  -- Remove and deallocate an Agar object class.
  --
  procedure Destroy_Class (Class : Class_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_DestroyClass";

  --
  -- Modify the initialization procedure of a class.
  --
  procedure Class_Set_Init
    (Class     : in Class_not_null_Access;
     Init_Func : in Init_Func_Access);
  function Class_Set_Init
    (Class     : in Class_not_null_Access;
     Init_Func : in Init_Func_Access) return Init_Func_Access;

  --
  -- Modify the reset procedure of a class.
  --
  procedure Class_Set_Reset
    (Class      : in Class_not_null_Access;
     Reset_Func : in Reset_Func_Access);
  function Class_Set_Reset
    (Class      : in Class_not_null_Access;
     Reset_Func : in Reset_Func_Access) return Reset_Func_Access;
  
  --
  -- Modify the finalization procedure of a class.
  --
  procedure Class_Set_Destroy
    (Class        : in Class_not_null_Access;
     Destroy_Func : in Destroy_Func_Access);
  function Class_Set_Destroy
    (Class        : in Class_not_null_Access;
     Destroy_Func : in Destroy_Func_Access) return Destroy_Func_Access;
  
  --
  -- Modify the deserialization function of a class.
  --
  procedure Class_Set_Load
    (Class     : in Class_not_null_Access;
     Load_Func : in Load_Func_Access);
  function Class_Set_Load
    (Class     : in Class_not_null_Access;
     Load_Func : in Load_Func_Access) return Load_Func_Access;

  --
  -- Modify the serialization function of a class.
  --
  procedure Class_Set_Save
    (Class     : in Class_not_null_Access;
     Save_Func : in Save_Func_Access);
  function Class_Set_Save
    (Class     : in Class_not_null_Access;
     Save_Func : in Save_Func_Access) return Save_Func_Access;
  
  --
  -- Modify the (application-specific) edit function of a class.
  --
  procedure Class_Set_Edit
    (Class     : in Class_not_null_Access;
     Edit_Func : in Edit_Func_Access);
  function Class_Set_Edit
    (Class     : in Class_not_null_Access;
     Edit_Func : in Edit_Func_Access) return Edit_Func_Access;

  --
  -- Register a new namespace and prefix. This is used for expanding inheritance
  -- hierarchy strings such as "Agar(Foo:Bar)" to "AG_Foo:AG_Bar".
  -- For example, ("Agar", "AG_", "http://libagar.org/").
  --
  procedure Register_Namespace
    (Name   : in String;
     Prefix : in String;
     URL    : in String);
  procedure Unregister_Namespace
    (Name : in String);
  
  --
  -- Return access to the registered class described by a hierarchy string.
  --
  function Lookup_Class (Class : in String) return Class_Access;

  --
  -- Load any module (and register any class) required to satisfy the given
  -- inheritance hierarchy. Modules are specified as a comma-separated list
  -- following '@'. For example: "MySuper:MySubclass@mymodule" specifies
  -- that Load_Class should scan the registered Module Directories for a DSO
  -- such as mymodule.so. If found, it expects to find a symbol "mySubclass"
  -- pointing to an initialized Class describing MySubclass.
  --
  function Load_Class (Class : in String) return Class_Access;

  --
  -- Add or remove directories to the search path used by Load_Class.
  --
  procedure Register_Module_Directory (Path : in String);
  procedure Unregister_Module_Directory (Path : in String);

  --
  -- Test if Object is a member of the class described by a String pattern.
  -- Pattern describes an inheritance hierarchy with possible wildcards ("*").
  -- "Vehicle:Car"   <= True if this is a Car (but not a subclass of Car).
  -- "Vehicle:Car:*" <= True if this is a Car (or a subclass of Car).
  --
  function Is_Of_Class
    (Object  : in Object_not_null_Access;
     Pattern : in String) return Boolean;
  function Is_A
    (Object  : in Object_not_null_Access;
     Pattern : in String) return Boolean;

  --
  -- Test if an object is being referenced by another object in the same VFS.
  --
  function In_Use (Object : in Object_not_null_Access) return Boolean;

  --
  -- Free an Object instance from memory.
  --
  procedure Destroy (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ObjectDestroy";

  --
  -- Restore the Object to an initial state. Invoked by both Load and Destroy.
  --
  procedure Reset (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ObjectReset";

  --
  -- Clear the registered event handlers and variables.
  --
  procedure Free_Events (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ObjectFreeEvents";
  procedure Free_Variables (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ObjectFreeVariables";

  --
  -- Detach and Destroy all child objects.
  --
  procedure Free_Children (Object : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ObjectFreeChildren";

  --
  -- Load the state of an object from serialized storage.
  --
  -- If File is not given, load from the application's default data directory
  -- (see: Agar.Init.Init_Core arguments Program_Name, Create_Directory).
  --
  function Load (Object : in Object_not_null_Access) return Boolean;
  function Load
    (Object : in Object_not_null_Access;
     File   : in String) return Boolean;

  --
  -- Load an Object's data part only (ignoring the generic Object part).
  --
  function Load_Data (Object : in Object_not_null_Access) return Boolean;
  function Load_Data
    (Object : in Object_not_null_Access;
     File   : in String) return Boolean;

  --
  -- Load an Object's generic part only (ignoring its data part).
  --
  function Load_Generic (Object : in Object_not_null_Access) return Boolean;
  function Load_Generic
    (Object : in Object_not_null_Access;
     File   : in String) return Boolean;

  --
  -- Save the state of an object to serialized storage.
  --
  -- If File is not given, save to the application's default data directory
  -- (see: Agar.Init.Init_Core arguments Program_Name, Create_Directory).
  --
  function Save (Object : in Object_not_null_Access) return Boolean;
  function Save
    (Object : in Object_not_null_Access;
     File   : in String) return Boolean;

  --
  -- Save the state of an Object and all of its descendants to the default
  -- data directory.
  --
  function Save_All (Object : in Object_not_null_Access) return Boolean;

  --
  -- Transfer a serialized representation of an object from/to an Agar data
  -- source (which could be a file, memory, network stream, etc).
  --
  function Serialize
    (Object : in Object_not_null_Access;
     Source : in DS.Data_Source_not_null_Access) return Boolean;
  function Unserialize
    (Object : in Object_not_null_Access;
     Source : in DS.Data_Source_not_null_Access) return Boolean;

  --
  -- Try reading an Agar object signature from a data source. If successful
  -- (True), return signature information in Header.
  --
  function Read_Header
    (Source : in DS.Data_Source_not_null_Access;
     Header : in Header_Access) return Boolean;

  --
  -- Page the data of an object in or out of serialized storage.
  --
  function Page_In (Object : in Object_not_null_Access) return Boolean;
  function Page_Out (Object : in Object_not_null_Access) return Boolean;

  --
  -- Set a callback routine for handling an event. The function forms
  -- returns an Event access which can be used to specify arguments
  -- (see Agar.Event). Set_Event does not allow more than one callback
  -- per event.
  --
  -- Async     => True : Process events in a separate thread.
  -- Propagate => True : Broadcast events to all child objects.
  --
  function Set_Event
    (Object    : in Object_not_null_Access;
     Event     : in String;
     Func      : in Event_Func_Access;
     Async     : in Boolean := False;
     Propagate : in Boolean := False) return EV.Event_not_null_Access;
  procedure Set_Event
    (Object    : in Object_not_null_Access;
     Event     : in String;
     Func      : in Event_Func_Access;
     Async     : in Boolean := False;
     Propagate : in Boolean := False);
  
  --
  -- Variant of Set_Event which allows more than one callback per Event.
  -- Instead of replacing an existing Event, Add_Event appends the callback
  -- the list. All registered callbacks will be invoked by Post_Event.
  --
  function Add_Event
    (Object    : in Object_not_null_Access;
     Event     : in String;
     Func      : in Event_Func_Access;
     Async     : in Boolean := False;
     Propagate : in Boolean := False) return EV.Event_not_null_Access;
  procedure Add_Event
    (Object    : in Object_not_null_Access;
     Event     : in String;
     Func      : in Event_Func_Access;
     Async     : in Boolean := False;
     Propagate : in Boolean := False);

  --
  -- Post an Event to a target Object.
  --
  procedure Post_Event
    (Object : in Object_not_null_Access;
     Event  : in String);
  procedure Post_Event
    (Object : in Object_not_null_Access;
     Event  : in EV.Event_not_null_Access);
  
  --
  -- Log object name and message to the console for debugging purposes.
  --
  procedure Debug
    (Object  : in Object_Access;
     Message : in String);
  
  --
  -- Attach an initialized timer to an object and activate it. The callback
  -- routine will be executed after the timer expires in Interval milliseconds.
  -- Timer will be restarted or deleted based on the callback's return value.
  --
  function Add_Timer
    (Object   : in Object_Access;
     Timer    : in TMR.Timer_not_null_Access;
     Interval : in Interfaces.Unsigned_32;
     Func     : in TMR.Timer_Callback) return Boolean;
  
  --
  -- Auto-allocated alternative to Init_Timer and Add_Timer. The timer structure
  -- will be auto allocated and freed implicitely.
  --
  function Add_Timer
    (Object   : in Object_Access;
     Interval : in Interfaces.Unsigned_32;
     Func     : in TMR.Timer_Callback) return TMR.Timer_Access;
 
  --
  -- Cancel and delete an active timer.
  --
  procedure Delete_Timer
    (Object : in Object_not_null_Access;
     Timer  : in TMR.Timer_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_DelTimer";

  --
  -- Evaluate whether a variable is set.
  --
  function Defined
    (Object   : in Object_not_null_Access;
     Variable : in String) return Boolean;

  --
  -- Compare two variables with no dereference. Discrete types are compared
  -- by value. Strings are compared case-sensitively. Reference types are
  -- compared by their pointer value.
  --
  function "=" (Left, Right : in Variable_not_null_Access) return Boolean;

  private

  --
  -- AG_Object(3)
  --

  function AG_ObjectNew
    (Parent : in System.Address;
     Name   : in CS.chars_ptr;
     Class  : in Class_not_null_Access) return Object_Access
    with Import, Convention => C, Link_Name => "AG_ObjectNew";
  
  procedure AG_ObjectInit
    (Object : in Object_not_null_Access;
     Class  : in Class_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ObjectInit";
  
  procedure AG_ObjectAttach
    (Parent : in Object_Access;
     Child  : in Object_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_ObjectAttach";
  
  function AG_ObjectFindS
    (Root : in Object_not_null_Access;
     Path : in CS.chars_ptr) return Object_Access
    with Import, Convention => C, Link_Name => "AG_ObjectFindS";
  
  function AG_ObjectFindParent
    (Object : in Object_not_null_Access;
     Name   : in CS.chars_ptr;
     Class  : in CS.chars_ptr) return Object_Access
    with Import, Convention => C, Link_Name => "AG_ObjectFindParent";
    
  function AG_ObjectFindChild
    (Parent : in Object_not_null_Access;
     Name   : in CS.chars_ptr) return Object_Access
    with Import, Convention => C, Link_Name => "ag_object_find_child";
  
  function AG_ObjectCopyName
    (Object : in Object_not_null_Access;
     Buffer : in System.Address;
     Size   : in AG_Size) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectCopyName";

  procedure AG_ObjectSetNameS
    (Object : in Object_not_null_Access;
     Name   : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_ObjectSetNameS";

  procedure AG_ObjectGenName
    (Object : in Object_not_null_Access;
     Class  : in Class_not_null_Access;
     Buffer : in System.Address;
     Size   : in AG_Size)
    with Import, Convention => C, Link_Name => "AG_ObjectGenName";

  procedure AG_ObjectGenNamePfx
    (Object : in Object_not_null_Access;
     Prefix : in CS.chars_ptr;
     Buffer : in System.Address;
     Size   : in AG_Size)
    with Import, Convention => C, Link_Name => "AG_ObjectGenNamePfx";

  function AG_CreateClass
    (Hierarchy   : in CS.chars_ptr;
     Object_Size : in AG_Size;
     Class_Size  : in AG_Size;
     Major       : in C.unsigned;
     Minor       : in C.unsigned) return Class_not_null_Access
    with Import, Convention => C, Link_Name => "AG_CreateClass";
  
  procedure AG_ClassSetInit
    (Class     : in Class_not_null_Access;
     Init_Func : in Init_Func_Access)
    with Import, Convention => C, Link_Name => "AG_ClassSetInit";
  function AG_ClassSetInit
    (Class     : in Class_not_null_Access;
     Init_Func : in Init_Func_Access) return Init_Func_Access
    with Import, Convention => C, Link_Name => "AG_ClassSetInit";
  
  procedure AG_ClassSetReset
    (Class      : in Class_not_null_Access;
     Reset_Func : in Reset_Func_Access)
    with Import, Convention => C, Link_Name => "AG_ClassSetReset";
  function AG_ClassSetReset
    (Class      : in Class_not_null_Access;
     Reset_Func : in Reset_Func_Access) return Reset_Func_Access
    with Import, Convention => C, Link_Name => "AG_ClassSetReset";
    
  procedure AG_ClassSetDestroy
    (Class        : in Class_not_null_Access;
     Destroy_Func : in Destroy_Func_Access)
    with Import, Convention => C, Link_Name => "AG_ClassSetDestroy";
  function AG_ClassSetDestroy
    (Class        : in Class_not_null_Access;
     Destroy_Func : in Destroy_Func_Access) return Destroy_Func_Access
    with Import, Convention => C, Link_Name => "AG_ClassSetDestroy";
    
  procedure AG_ClassSetLoad
    (Class     : in Class_not_null_Access;
     Load_Func : in Load_Func_Access)
    with Import, Convention => C, Link_Name => "AG_ClassSetLoad";
  function AG_ClassSetLoad
    (Class     : in Class_not_null_Access;
     Load_Func : in Load_Func_Access) return Load_Func_Access
    with Import, Convention => C, Link_Name => "AG_ClassSetLoad";
    
  procedure AG_ClassSetSave
    (Class     : in Class_not_null_Access;
     Save_Func : in Save_Func_Access)
    with Import, Convention => C, Link_Name => "AG_ClassSetSave";
  function AG_ClassSetSave
    (Class     : in Class_not_null_Access;
     Save_Func : in Save_Func_Access) return Save_Func_Access
    with Import, Convention => C, Link_Name => "AG_ClassSetSave";
    
  procedure AG_ClassSetEdit
    (Class     : in Class_not_null_Access;
     Edit_Func : in Edit_Func_Access)
    with Import, Convention => C, Link_Name => "AG_ClassSetEdit";
  function AG_ClassSetEdit
    (Class     : in Class_not_null_Access;
     Edit_Func : in Edit_Func_Access) return Edit_Func_Access
    with Import, Convention => C, Link_Name => "AG_ClassSetEdit";
  
  procedure AG_RegisterNamespace
    (Name   : in CS.chars_ptr;
     Prefix : in CS.chars_ptr;
     URL    : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_RegisterNamespace";

  procedure AG_UnregisterNamespace (Name : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_UnregisterNamespace";

  function AG_LookupClass (Class : in CS.chars_ptr) return Class_Access
    with Import, Convention => C, Link_Name => "AG_LookupClass";

  function AG_LoadClass (Class : in CS.chars_ptr) return Class_Access
    with Import, Convention => C, Link_Name => "AG_LoadClass";

  procedure AG_RegisterModuleDirectory (Path : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_RegisterModuleDirectory";

  procedure AG_UnregisterModuleDirectory (Path : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_UnregisterModuleDirectory";

  function AG_OfClass
    (Object  : in Object_not_null_Access;
     Pattern : in CS.chars_ptr) return C.int
    with Import, Convention => C, Link_Name => "ag_of_class";

  function AG_ObjectInUse (Object : in Object_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectInUse";

  function AG_ObjectLoad (Object : in Object_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectLoad";

  function AG_ObjectLoadFromFile
    (Object : in Object_not_null_Access;
     File   : in CS.chars_ptr) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectLoadFromFile";

  function AG_ObjectLoadData (Object : in Object_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectLoadData";

  function AG_ObjectLoadDataFromFile
    (Object : in Object_not_null_Access;
     File   : in CS.chars_ptr) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectLoadDataFromFile";

  function AG_ObjectLoadGeneric (Object : in Object_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectLoadGeneric";

  function AG_ObjectLoadGenericFromFile
    (Object : in Object_not_null_Access;
     File   : in CS.chars_ptr) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectLoadGenericFromFile";

  function AG_ObjectSave (Object : in Object_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectSave";

  function AG_ObjectSaveAll (Object : in Object_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectSaveAll";

  function AG_ObjectSaveToFile
    (Object : in Object_not_null_Access;
     File   : in CS.chars_ptr) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectSaveToFile";

  function AG_ObjectSerialize
    (Object : in Object_not_null_Access;
     Source : in DS.Data_Source_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectSerialize";

  function AG_ObjectUnserialize
    (Object : in Object_not_null_Access;
     Source : in DS.Data_Source_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectUnserialize";

  function AG_ObjectReadHeader
    (Object : in DS.Data_Source_not_null_Access;
     Header : in Header_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectReadHeader";

  function AG_ObjectPageIn (Object : in Object_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectPageIn";

  function AG_ObjectPageOut (Object : in Object_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_ObjectPageOut";
  
  procedure AG_Debug
    (Object  : in Object_Access;
     Format  : in CS.chars_ptr;
     Message : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_Debug";
  
  --
  -- AG_Event(3)
  --
  
  function AG_SetEvent
    (Object    : in Object_not_null_Access;
     Event     : in CS.chars_ptr;
     Func      : in Event_Func_Access;
     Format    : in CS.chars_ptr) return EV.Event_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SetEvent";
  
  procedure AG_SetEvent
    (Object    : in Object_not_null_Access;
     Event     : in CS.chars_ptr;
     Func      : in Event_Func_Access;
     Format    : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_SetEvent";
  
  function AG_AddEvent
    (Object : in Object_not_null_Access;
     Event  : in CS.chars_ptr;
     Func   : in Event_Func_Access;
     Format : in CS.chars_ptr) return EV.Event_not_null_Access
    with Import, Convention => C, Link_Name => "AG_AddEvent";
  procedure AG_AddEvent
    (Object : in Object_not_null_Access;
     Event  : in CS.chars_ptr;
     Func   : in Event_Func_Access;
     Format : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_AddEvent";
  
  function AG_PostEvent
    (Object : in Object_not_null_Access;
     Event  : in CS.chars_ptr;
     Format : in CS.chars_ptr) return EV.Event_not_null_Access
    with Import, Convention => C, Link_Name => "AG_PostEvent";
  procedure AG_PostEvent
    (Object : in Object_not_null_Access;
     Event  : in CS.chars_ptr;
     Format : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_PostEvent";
  
  procedure AG_PostEventByPtr
    (Object : in Object_not_null_Access;
     Event  : in EV.Event_Access;
     Format : in CS.chars_ptr)
    with Import, Convention => C, Link_Name => "AG_PostEventByPtr";
  
  --
  -- AG_Timer(3)
  --
  
  function AG_AddTimer
    (Object   : in Object_Access;
     Timer    : in TMR.Timer_not_null_Access;
     Interval : in Interfaces.Unsigned_32;
     Func     : in TMR.Timer_Callback;
     Flags    : in C.unsigned;
     Format   : in CS.chars_ptr) return C.int
    with Import, Convention => C, Link_Name => "AG_AddTimer";
  
  function AG_AddTimerAuto
    (Object   : in Object_Access;
     Interval : in Interfaces.Unsigned_32;
     Func     : in TMR.Timer_Callback;
     Format   : in CS.chars_ptr) return TMR.Timer_Access
    with Import, Convention => C, Link_Name => "AG_AddTimerAuto";

  --------------------
  -- AG_Variable(3) --
  --------------------

  function AG_Defined
    (Object : in Object_not_null_Access;
     Name   : in CS.chars_ptr) return C.int
    with Import, Convention => C, Link_Name => "ag_defined";

  procedure AG_CopyVariable
    (Destination : in Variable_not_null_Access;
     Source      : in Variable_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_CopyVariable";

  procedure AG_Unset
    (Object : in Object_not_null_Access;
     Name   : in System.Address)
    with Import, Convention => C, Link_Name => "AG_Unset";

  procedure AG_GetUint8
    (Object : in Object_not_null_Access;
     Name   : in System.Address)
    with Import, Convention => C, Link_Name => "AG_GetUint8";

  function AG_SetUint8
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : in Interfaces.Unsigned_8) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SetUint8";

  function AG_BindUint8
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : access Interfaces.Unsigned_8) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_BindUint8";

  function AG_GetSint8
    (Object : in System.Address;
     Name   : in System.Address) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_GetSint8";

  function AG_SetSint8
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : in Signed_8) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SetSint8";

  function AG_BindSint8
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : access Signed_8) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_BindSint8";

  procedure AG_GetUint16
    (Object : in System.Address;
     Name   : in System.Address)
    with Import, Convention => C, Link_Name => "AG_GetUint16";

  function AG_SetUint16
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : in Interfaces.Unsigned_16) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SetUint16";

  function AG_BindUint16
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : access Interfaces.Unsigned_16) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_BindUint16";

  function AG_GetSint16
    (Object : in System.Address;
     Name   : in System.Address) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_GetSint16";

  function AG_SetSint16
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : in Signed_16) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SetSint16";

  function AG_BindSint16
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : access Signed_16) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_BindSint16";

  procedure AG_GetUint32
    (Object : in System.Address;
     Name   : in System.Address)
    with Import, Convention => C, Link_Name => "AG_GetUint32";

  function AG_SetUint32
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : in Interfaces.Unsigned_32) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SetUint32";

  function AG_BindUint32
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : access Interfaces.Unsigned_32) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_BindUint32";

  function AG_GetSint32
    (Object : in System.Address;
     Name   : in System.Address) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_GetSint32";

  function AG_SetSint32
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : in Signed_32) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SetSint32";

  function AG_BindSint32
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : access Signed_32) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_BindSint32";

#if HAVE_64BIT
  procedure AG_GetUint64
    (Object : in System.Address;
     Name   : in System.Address)
    with Import, Convention => C, Link_Name => "AG_GetUint64";

  function AG_SetUint64
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : in Interfaces.Unsigned_64) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SetUint64";

  function AG_BindUint64
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : access Interfaces.Unsigned_64) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_BindUint64";

  function AG_GetSint64
    (Object : in System.Address;
     Name   : in System.Address) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_GetSint64";

  function AG_SetSint64
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : in Signed_64) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SetSint64";

  function AG_BindSint64
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : access Signed_64) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_BindSint64";
#end if;

#if HAVE_FLOAT
  function AG_GetFloat
    (Object : in System.Address;
     Name   : in System.Address) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_GetFloat";

  function AG_SetFloat
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : in Float) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SetFloat";

  function AG_BindFloat
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : access Float) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_BindFloat";

  function AG_GetDouble
    (Object : in System.Address;
     Name   : in System.Address) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_GetDouble";

  function AG_SetDouble
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : in Double) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SetDouble";

  function AG_BindDouble
    (Object : in System.Address;
     Name   : in System.Address;
     Value  : access Double) return Variable_not_null_Access
    with Import, Convention => C, Link_Name => "AG_BindDouble";
#end if;

  function AG_GetString
    (Object : in System.Address;
     Name   : in System.Address;
     Buffer : in System.Address;
     Size   : in AG_Size) return AG_Size
    with Import, Convention => C, Link_Name => "AG_GetString";

  function AG_GetStringDup
    (Object : in System.Address;
     Name   : in System.Address) return System.Address
    with Import, Convention => C, Link_Name => "AG_GetStringDup";

  function AG_SetString
    (Variable : in Variable_not_null_Access;
     Name     : in System.Address;
     Data     : in System.Address) return Variable_Access
    with Import, Convention => C, Link_Name => "AG_SetString";

  function AG_BindString
    (Variable : in Variable_not_null_Access;
     Name     : in System.Address;
     Data     : in System.Address;
     Size     : in AG_Size) return Variable_Access
    with Import, Convention => C, Link_Name => "AG_BindString";

  function AG_GetPointer
    (Object : in System.Address;
     Name   : in System.Address) return System.Address
    with Import, Convention => C, Link_Name => "AG_GetPointer";

  function AG_SetPointer
    (Variable : in Variable_not_null_Access;
     Name     : in System.Address;
     Data     : in System.Address) return Variable_Access
    with Import, Convention => C, Link_Name => "AG_SetPointer";

  function AG_BindPointer
    (Variable : in Variable_not_null_Access;
     Name     : in System.Address;
     Data     : access System.Address) return Variable_Access
    with Import, Convention => C, Link_Name => "AG_BindPointer";
  
  function AG_CompareVariables
    (Left, Right : in Variable_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_CompareVariables";
  
end Agar.Object;
