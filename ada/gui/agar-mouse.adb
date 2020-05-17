------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                           A G A R  . M O U S E                           --
--                                 B o d y                                  --
--                                                                          --
-- Copyright (c) 2019 Julien Nadeau Carriere (vedge@csoft.net)         --
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

package body Agar.Mouse is

  --
  -- Return the current cursor position and button state.
  --
  procedure Get_Mouse_State
    (Mouse   : in     Mouse_Device_not_null_Access;
     Buttons :    out Interfaces.Unsigned_8;
     X,Y     :    out Natural)
  is
    St_X, St_Y : aliased C.int;
  begin
    Buttons := AG_MouseGetState
      (Mouse => Mouse,
       X     => St_X'Access,
       Y     => St_Y'Access);
    X := Natural(St_X);
    Y := Natural(St_Y);
  end;

  --
  -- Update the internal mouse state following a motion event.
  --
  procedure Mouse_Motion_Update
    (Mouse   : in Mouse_Device_not_null_Access;
     X,Y     : in Natural) is
  begin
    AG_MouseMotionUpdate
      (Mouse => Mouse,
       X     => C.int(X),
       Y     => C.int(Y));
  end;
  
  --
  -- Update the internal mouse state following a mouse button event.
  --
  procedure Mouse_Button_Update
    (Mouse  : in Mouse_Device_not_null_Access;
     Action : in Mouse_Button_Action;
     Button : in Mouse_Button) is
  begin
    AG_MouseButtonUpdate
      (Mouse  => Mouse,
       Action => Action,
       Button => Mouse_Button'Pos(Button));
  end;

end Agar.Mouse;
