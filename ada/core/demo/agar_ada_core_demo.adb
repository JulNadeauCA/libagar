----------------------------------------------------
-- agar_ada_core_demo.adb: Agar Ada bindings demo --
----------------------------------------------------
with Agar;
with Agar.Init;
with Agar.Error;
with Agar.Object; use Agar.Object;
with Agar.Event;
with Agar.DSO;
with Agar.Types; use Agar.Types;
with Ada.Text_IO;
with System;
with myatexit;
with myeventhandler;
with Animal;
with Ada.Real_Time; use Ada.Real_Time;

procedure agar_ada_core_demo is
  package T_IO renames Ada.Text_IO;
  package EV renames Agar.Event;
  package RT renames Ada.Real_Time;

  Major : Natural;
  Minor : Natural;
  Patch : Natural;

  My_Parent    : Object_Access;
  My_Child_1   : Object_Access;
  My_Child_2   : Object_Access;
  Animal_Class : Class_Access;
  Cow          : Object_Access;
  Event        : EV.Event_Access;
  Epoch        : constant RT.Time := RT.Clock;

begin

  if not Agar.Init.Init_Core
    (Program_Name     => "agar_ada_core_demo",
     Create_Directory => True)
  then
    raise program_error with Agar.Error.Get_Error;
  end if;

  T_IO.Put_Line("Agar-Core initialized in " &
    Duration'Image(RT.To_Duration(RT.Clock - Epoch)) & "s");

  -- Register a test atexit callback.
  Agar.Init.At_Exit(myatexit.atexit'Access);

  -- Print Agar's version number.
  Agar.Init.Get_Version(Major, Minor, Patch);
  T_IO.Put_Line ("Agar version" &
     Integer'Image(Major) & " ." &
     Integer'Image(Minor) & " ." &
     Integer'Image(Patch));
  
  T_IO.Put_Line ("Memory model: " & Natural'Image(AG_MODEL));

  -- Register the Agar object class "Animal" specified in animal.ads.
  T_IO.Put_Line("Registering Animal class (" &
    Natural'Image(Animal.Animal'Size / System.Storage_Unit) & " bytes)");
  Animal_Class := Animal.Create_Class;

  -- Create an instance the Animal class.
  Cow := New_Object(Animal_Class);
  Set_Name(Cow, "Cow");
  Debug(Cow, "Moo!");

  -- Create a generic AG_Object(3) instance.
  My_Parent := New_Object(Lookup_Class("AG_Object"));
  Set_Name(My_Parent, "My_Test_Object");

  -- Access the class description of the object.
  T_IO.Put_Line("Object is" &
    Natural'Image(Natural(My_Parent.Class.Size)) & " bytes");

  -- Configure an event handler for `some-event' and pass it some arguments.
  Event := Set_Event
    (Object => My_Parent,
     Event  => "some-event",
     Func   => myeventhandler.Some_Event'Access);
  EV.Push_String  (Event, "This is a string argument");   -- untagged arguments
  EV.Push_Float   (Event, 1234.0);
  EV.Push_Natural (Event, "width", 640);                  -- tagged arguments
  EV.Push_Natural (Event, "height", 480);
  
  -- Raise `some-event' by name.
  T_IO.Put_Line("Raising some-event by name");
  Post_Event (My_Parent, "some-event");
  
  -- Raise `some-event' by access (as returned by Set_Event).
  T_IO.Put_Line("Raising some-event by access");
  Post_Event (My_Parent, Event);
 
  --
  -- Raise `some-event' and pass the callback procedure some extra arguments
  -- on top of the existing argument list constructed by Set_Event.
  --
  --Event := Prepare_Event (My_Parent, "Some-Event");
  --EV.Push_Integer (Event, "timestamp", Ada.Real_Time.Clock - Epoch);
  --EV.Push_String  (Event, "Hello there!");
  --Post_Event (Event);
  
  -- Create two child objects under My_Parent.
  T_IO.Put_Line("Creating child objects");
  My_Child_1 := New_Object(My_Parent, "My_Child_1", Lookup_Class("AG_Object"));
  My_Child_2 := New_Object(My_Parent, "My_Child_2", Lookup_Class("AG_Object"));
 
  -- Objects can send events to each other.
  Post_Event
    (Object => My_Child_2,
     Event  => "Ping");

  -- Propagate makes events broadcast to the object's descendants.
  Set_Event
    (Object    => My_Parent,
     Event     => "Ping",
     Func      => myeventhandler.Ping'Access,
     Async     => False,
     Propagate => True);
  Post_Event
    (Object => My_Parent,
     Event  => "Ping");

  T_IO.Put_Line("My_Parent path = " & Get_Name(My_Parent));
  T_IO.Put_Line("My_Child_1 path = " & Get_Name(My_Child_1));
  T_IO.Put_Line("My_Child_2 path = " & Get_Name(My_Child_2));

  -- The Parent and Root members of an object are protected by the parent VFS.
  Lock_VFS(My_Child_2);
  T_IO.Put_Line("Parent of My_Child_2 is = " & Get_Name(My_Child_2.Parent));
  T_IO.Put_Line("Root of My_Child_2 is = "   & Get_Name(My_Child_2.Root));
  Unlock_VFS(My_Child_2);
  
  -- Serialize an object to a file.
  if not Save(Cow, "Cow.obj") then
    raise program_error with Agar.Error.Get_Error;
  end if;
  T_IO.Put_Line("Saved Cow to Cow.obj");
  if not Save(My_Parent, "My_Parent.obj") then
    raise program_error with Agar.Error.Get_Error;
  end if;
  T_IO.Put_Line("Saved My_Parent to My_Parent.obj");

  -- Serialize the entire VFS to the default data directory.
  if not Save_All(My_Parent) then
    raise program_error with Agar.Error.Get_Error;
  end if;
  T_IO.Put_Line("Saved My_Parent to VFS");

  -- Register a module directory and list all available DSOs.
  Agar.Object.Register_Module_Directory ("/tmp/dsotest");
  declare
    DSO_List : constant Agar.DSO.DSO_List := Agar.DSO.Get_List;
  begin
    for DSO of DSO_List loop
      T_IO.Put("Available DSO: ");
      T_IO.Put_Line(DSO);
    end loop;
  end;

  Detach(My_Child_1);
  Detach(My_Child_2);
  Destroy(My_Child_1);
  Destroy(My_Child_2);
  Destroy(My_Parent);

  Destroy(Cow);
  Destroy_Class(Animal_Class);
  
  T_IO.Put_Line("Exiting after " &
    Duration'Image(RT.To_Duration(RT.Clock - Epoch)) & "s");

  Agar.Init.Quit;
end agar_ada_core_demo;
