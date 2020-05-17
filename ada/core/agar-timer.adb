------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                           A G A R . T I M E R                            --
--                                 B o d y                                  --
--                                                                          --
-- Copyright (c) 2018-2019, Julien Nadeau Carriere (vedge@csoft.net)        --
--                                                                          --
-- Permission to use, copy, modify, and/or distribute this software for any --
-- purpose with or without fee is hereby granted, provided that the above   --
-- copyright notice and this permission notice appear in all copies.        --
--                                                                          --
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES --
-- WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         --
-- MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  --
-- ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   --
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    --
-- ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  --
-- OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           --
------------------------------------------------------------------------------
package body Agar.Timer is
  use type C.int;

  procedure Init_Timer
    (Timer : in Timer_not_null_Access;
     Name  : in String)
  is
    Ch_Name : aliased C.char_array := C.To_C(Name);
  begin
    AG_InitTimer
      (Timer => Timer,
       Name  => CS.To_Chars_Ptr(Ch_Name'Unchecked_Access),
       Flags => 0);
  end;
  
  function Add_Timer
    (Timer    : in Timer_not_null_Access;
     Interval : in Interfaces.Unsigned_32;
     Func     : in Timer_Callback) return Boolean
  is
  begin
    return 0 = AG_AddTimer
      (Object   => System.Null_Address,
       Timer    => Timer,
       Interval => Interval,
       Func     => Func,
       Flags    => 0,
       Format   => CS.Null_Ptr);
  end;
  
  function Add_Timer
    (Interval : in Interfaces.Unsigned_32;
     Func     : in Timer_Callback) return Boolean
  is
  begin
    return null /= AG_AddTimerAuto
      (Object   => System.Null_Address,
       Interval => Interval,
       Func     => Func,
       Format   => CS.Null_Ptr);
  end;
  
end Agar.Timer;
