------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                    A G A R  . I N P U T _ D E V I C E                    --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces.C;
with Interfaces.C.Strings;
with Agar.Object;
with Agar.Event;
with System;

--
-- Generic input device
--

package Agar.Input_Device is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;
  use type C.unsigned;

  type Input_Device is limited record
    Super   : aliased Agar.Object.Object; -- Input_Device
    Flags   : C.unsigned;
    Driver  : System.Address;             -- Agar.Driver.Driver_Access
    Descr   : CS.chars_ptr;               -- Long description
    Event_Q : Agar.Event.Event_Queue;     -- Queued events
  end record
    with Convention => C;

  type Input_Device_Access is access all Input_Device with Convention => C;
  subtype Input_Device_not_null_Access is not null Input_Device_Access;

end Agar.Input_Device;
