------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                          A G A R  . W I D G E T                          --
--                                 B o d y                                  --
--                                                                          --
-- Copyright (c) 2018-2019 Julien Nadeau Carriere (vedge@csoft.net)         --
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
package body Agar.Widget is
  
  --
  -- Return the first visible widget intersecting a point or enclosing a
  -- rectangle (in view coordinates). Scan all drivers and return first match.
  --
  --function Find_At_Point
  --  (Class : in String;
  --   X,Y   : in Natural) return Widget_Access;
  --function Find_Enclosing_Rect
  --  (Class : in String;
  --   X,Y   : in Natural;
  --   W,H   : in Positive) return Widget_Access;

  --
  -- Set whether to accept (or deny) focused state.
  --
  procedure Set_Focusable
    (Widget : in Widget_not_null_Access;
     Enable : in Boolean)
  is
    Former_Status : aliased C.int;
  begin
    if Enable then
      Former_Status := AG_WidgetSetFocusable (Widget, C.int(1));
    else
      Former_Status := AG_WidgetSetFocusable (Widget, C.int(0));
    end if;
  end;

  --
  -- Set whether to accept (or deny) focused state (return previous).
  --
  function Set_Focusable
    (Widget : in Widget_not_null_Access;
     Enable : in Boolean) return Boolean
  is
  begin
    if Enable then
      return 1 = AG_WidgetSetFocusable (Widget, C.int(1));
    else
      return 1 = AG_WidgetSetFocusable (Widget, C.int(0));
    end if;
  end;

  --
  -- Focus on the widget (and implicitely its parents up to and including
  -- the parent window).
  --
  function Focus
    (Widget : in Widget_not_null_Access) return Boolean is
  begin
    return 0 = AG_WidgetFocus (Widget);
  end;
  
  --
  -- Return the topmost visible widget intersecting a display-coordinate point.
  --
  function Find_Widget_At_Point
    (Class : String;
     X,Y   : Natural) return Widget_Access
  is
    Ch_Class : aliased C.char_array := C.To_C(Class);
  begin
    return AG_WidgetFindPoint
        (Class => CS.To_Chars_Ptr(Ch_Class'Unchecked_Access),
         X     => C.int(X),
         Y     => C.int(Y));
  end;

  --
  -- Return topmost visible widget intersecting a display-coordinate rectangle.
  --
  function Find_Widget_Enclosing_Rect
    (Class         : String;
     X,Y           : Natural;
     Width, Height : Positive) return Widget_Access
  is
    Ch_Class : aliased C.char_array := C.To_C(Class);
  begin
    return AG_WidgetFindRect
      (Class  => CS.To_Chars_Ptr(Ch_Class'Unchecked_Access),
       X      => C.int(X),
       Y      => C.int(Y),
       Width  => C.int(Width),
       Height => C.int(Height));
  end;
  
  --
  -- Update the cached absolute display (rView) coordinates of a widget, and
  -- of its descendants. The Widget and its parent VFS must both be locked.
  --
  procedure Update_Coordinates
    (Widget : in Widget_not_null_Access;
     X      : in Natural;
     Y      : in Natural) is
  begin
    AG_WidgetUpdateCoords
      (Widget => Widget,
       X      => C.int(X),
       Y      => C.int(Y));
  end;
  
  --
  -- Free any Surface mapped to the given handle (as returned by Map_Surface).
  -- Delete any hardware texture associated with the surface.
  --
  procedure Unmap_Surface
    (Widget : in Widget_not_null_Access;
     Handle : in Surface_Handle) is
  begin
    Replace_Surface
      (Widget  => Widget,
       Handle  => Handle,
       Surface => null);
  end;
  
  --
  -- Blit the surface (or render the hardware texture) at Source:[Handle],
  -- at target coordinates X,Y relative to Widget.
  --
  -- Source may be different from Widget (i.e., Widgets may render other
  -- widgets' surfaces) as long as both widgets are in the same Window.
  --
  procedure Blit_Surface
    (Widget   : in Widget_not_null_Access;
     Source   : in Widget_not_null_Access;
     Handle   : in Surface_Handle;
     Src_Rect : in SU.Rect_Access := null;
     X,Y      : in Natural := 0) is
  begin
    AG_WidgetBlitFrom
      (Widget   => Widget,
       Source   => Source,
       Handle   => Handle,
       Src_Rect => Src_Rect,
       X        => C.int(X),
       Y        => C.int(Y));
  end;
  
  --
  -- Blit the surface (or render the hardware texture) at Widget:[Handle],
  -- at target coordinates X,Y relative to Widget.
  --
  procedure Blit_Surface
    (Widget   : in Widget_not_null_Access;
     Handle   : in Surface_Handle;
     Src_Rect : in SU.Rect_Access := null;
     X,Y      : in Natural := 0) is
  begin
    AG_WidgetBlitFrom
      (Widget   => Widget,
       Source   => Widget,
       Handle   => Handle,
       Src_Rect => Src_Rect,
       X        => C.int(X),
       Y        => C.int(Y));
  end;
  
  --
  -- Blit a Surface not managed by the Widget. This method is inefficient
  -- (no hardware acceleration) and should be avoided.
  --
  procedure Blit_Surface
    (Widget   : in Widget_not_null_Access;
     Surface  : in SU.Surface_not_null_Access;
     X,Y      : in Natural := 0) is
  begin
    AG_WidgetBlit
      (Widget   => Widget,
       Surface  => Surface,
       X        => C.int(X),
       Y        => C.int(Y));
  end;
  
  --
  -- Test whether widget is sensitive to view coordinates X,Y.
  --
  function Is_Sensitive
    (Widget : in Widget_not_null_Access;
     X,Y    : in Natural) return Boolean is
  begin
    return 1 = AG_WidgetSensitive
      (Widget => Widget,
       X      => C.int(X),
       Y      => C.int(Y));
  end;

  --
  -- Create a new mouse instance under a Driver.
  --
  function New_Mouse
    (Driver : in Driver_not_null_Access;
     Descr  : in String) return MSE.Mouse_Device_not_null_Access
  is
    Ch_Descr : aliased C.char_array := C.To_C(Descr);
  begin
    return AG_MouseNew
      (Driver => Driver,
       Descr  => CS.To_Chars_Ptr(Ch_Descr'Unchecked_Access));
  end;

  --
  -- Change the cursor if its coordinates overlap a registered cursor area.
  -- Generally called from window/driver code following a mouse motion event.
  --
  procedure Mouse_Cursor_Update
    (Window : in Window_not_null_Access;
     X,Y    : in Natural) is
  begin
    AG_MouseCursorUpdate
      (Window => Window,
       X      => C.int(X),
       Y      => C.int(Y));
  end;

  --
  -- Handle a mouse motion. Called from Driver code (agDrivers must be locked).
  --
  procedure Process_Mouse_Motion
    (Window    : in Window_not_null_Access;
     X,Y       : in Natural;
     Xrel,Yrel : in Integer;
     Buttons   : in MSE.Mouse_Button) is
  begin
    AG_ProcessMouseMotion
      (Window  => Window,
       X       => C.int(X),
       Y       => C.int(Y),
       Xrel    => C.int(Xrel),
       Yrel    => C.int(Yrel),
       Buttons => Buttons);
  end;

  --
  -- Handle a mouse button release.
  -- Called from Driver code (agDrivers must be locked).
  --
  procedure Process_Mouse_Button_Up
    (Window    : in Window_not_null_Access;
     X,Y       : in Natural;
     Button    : in MSE.Mouse_Button) is
  begin
    AG_ProcessMouseButtonUp
      (Window => Window,
       X      => C.int(X),
       Y      => C.int(Y),
       Button => Button);
  end;

  --
  -- Handle a mouse button press.
  -- Called from Driver code (agDrivers must be locked).
  --
  procedure Process_Mouse_Button_Down
    (Window    : in Window_not_null_Access;
     X,Y       : in Natural;
     Button    : in MSE.Mouse_Button) is
  begin
    AG_ProcessMouseButtonDown
      (Window => Window,
       X      => C.int(X),
       Y      => C.int(Y),
       Button => Button);
  end;

end Agar.Widget;
