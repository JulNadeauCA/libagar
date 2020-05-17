with agar.core.event;

package keyevent_callbacks is
  package gui_event renames agar.core.event;

  procedure keydown (event : gui_event.event_access_t);
  procedure keyup (event : gui_event.event_access_t);
  procedure quit (event : gui_event.event_access_t);

  pragma convention (c, keydown);
  pragma convention (c, keyup);
  pragma convention (c, quit);
end keyevent_callbacks;
