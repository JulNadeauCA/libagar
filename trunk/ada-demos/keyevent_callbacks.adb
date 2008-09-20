with ada.text_io;

package body keyevent_callbacks is
  package io renames ada.text_io;

  procedure keydown (event : gui_event.event_access_t) is
    keyint : constant integer := gui_event.get_integer (event, 1);
    modint : constant integer := gui_event.get_integer (event, 2);
    keystr : constant string := integer'image (keyint);
    modstr : constant string := integer'image (modint);
  begin
    io.put_line ("keyevent: keydown key=" & keystr & " mod=" & modstr);
  end keydown;

  procedure keyup (event : gui_event.event_access_t) is
  begin
    io.put_line ("keyevent: keyup");
  end keyup;

  procedure quit (event : gui_event.event_access_t) is
  begin
    io.put_line ("keyevent: exiting");
    agar.core.quit;
  end quit;
end keyevent_callbacks;
