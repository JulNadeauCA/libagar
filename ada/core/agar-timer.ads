------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                           A G A R . T I M E R                            --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces;
with Interfaces.C;
with Interfaces.C.Strings;
with Agar.Event;
with System;

--
-- Interface to the Agar timer facility (AG_Timer(3) in C). Agar timers are
-- generally attached to a parent Agar object. Detaching the parent object from
-- its parent VFS has the effect of cancelling any active timers.
--
-- Unless Agar was initialized with the Software_Timers option, Agar timers
-- are implemented using the "best" available hardware interface on the current
-- platform (such as BSD kqueue(2), POSIX select(2) or Linux timerfd). The
-- Software_Timers option enforces a delay loop and timing wheel in software.
--

package Agar.Timer is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;
  package EV renames Agar.Event;
  
  TIMER_NAME_MAX : constant Natural := $AG_TIMER_NAME_MAX;

  type Timer_Private is array (1 .. $SIZEOF_AG_TimerPvt)
    of aliased Interfaces.Unsigned_8 with Convention => C;
  for Timer_Private'Size use $SIZEOF_AG_TimerPvt * System.Storage_Unit;
  
  type Timer_Name is array (1 .. TIMER_NAME_MAX)
    of aliased c.char with Convention => C;

  type    Timer;
  type    Timer_Access          is access all Timer with Convention => C;
  subtype Timer_not_null_Access is not null Timer_Access;
  type    Timer_Callback        is access function
    (Timer : Timer_Access;
     Event : EV.Event_Access) return Interfaces.Unsigned_32
     with Convention => C;
  subtype Timer_not_null_Callback is not null Timer_Callback;  

  type Timer is limited record
    Callback_Args   : EV.Event;			-- Callback arguments
    Callback_Func   : Timer_Callback;		-- Callback function
    Private_Data    : Timer_Private;		-- Private data
    Parent_Object   : System.Address;		-- Parent object
    Flags           : C.unsigned;
    Identifier      : C.int;			-- Unique identifier
    Expiration_Time : Interfaces.Unsigned_32;	-- Expires in this long (ticks)
    Interval        : Interfaces.Unsigned_32;	-- Timer interval (ticks)
    Name            : Timer_Name;		-- Optional name ID
  end record
    with Convention => C;

  procedure Init_Timer
    (Timer : in Timer_not_null_Access;
     Name  : in String);
  -- Initialize an (initially unattached) timer.

  function Add_Timer
    (Timer    : in Timer_not_null_Access;
     Interval : in Interfaces.Unsigned_32;
     Func     : in Timer_Callback) return Boolean;
  -- Start a global timer not attached to a specific object.
  
  function Add_Timer
    (Interval : in Interfaces.Unsigned_32;
     Func     : in Timer_Callback) return Boolean;
  -- Start a global, auto-allocated timer not attached to a specific object.

  private
  
  procedure AG_InitTimer
    (Timer : in Timer_not_null_Access;
     Name  : in CS.chars_ptr;
     Flags : in C.unsigned)
    with Import, Convention => C, Link_Name => "AG_InitTimer";
  
  function AG_AddTimer
    (Object   : in System.Address;
     Timer    : in Timer_not_null_Access;
     Interval : in Interfaces.Unsigned_32;
     Func     : in Timer_Callback;
     Flags    : in C.unsigned;
     Format   : in CS.chars_ptr) return C.int
    with Import, Convention => C, Link_Name => "AG_AddTimer";
  
  function AG_AddTimerAuto
    (Object   : in System.Address;
     Interval : in Interfaces.Unsigned_32;
     Func     : in Timer_Callback;
     Format   : in CS.chars_ptr) return Timer_Access
    with Import, Convention => C, Link_Name => "AG_AddTimerAuto";
  
end Agar.Timer;
