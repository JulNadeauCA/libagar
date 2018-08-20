with Ada.Text_IO;

package body myeventhandler is
  
  procedure Ping (Event : EV.Event_Access) is begin
    Ada.Text_IO.Put_Line ("Ping");
  end;

  procedure Some_Event (Event : EV.Event_Access) is
    My_String  : constant String  := EV.Get_String (Event, 1);       -- by index
    My_Float   : constant Float   := EV.Get_Float  (Event, 2);
    Width      : constant Natural := EV.Get_Natural(Event, "width");  -- by name
    Height     : constant Natural := EV.Get_Natural(Event, "height");
  begin
    Ada.Text_IO.Put_Line
      ("some-event: My_String=" & My_String & "; My_Float=" & Float'Image(My_Float));
    Ada.Text_IO.Put_Line(
      "some-event: Width=" & Natural'Image(Width) & "; Height=" & Natural'Image(Height));
  end;

end myeventhandler;
