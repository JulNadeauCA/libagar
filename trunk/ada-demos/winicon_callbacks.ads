with agar.core.event;

package winicon_callbacks is
  package gui_event renames agar.core.event;

  procedure quit (event : gui_event.event_access_t);

  pragma convention (c, quit);
end winicon_callbacks;
