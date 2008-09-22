with ada.text_io;

package body winicon_callbacks is
  package io renames ada.text_io;

  procedure quit (event : gui_event.event_access_t) is
  begin
    io.put_line ("winicon: exiting");
    agar.core.quit;
  end quit;
end winicon_callbacks;
